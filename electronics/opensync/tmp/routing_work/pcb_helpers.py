"""PCB geometry helpers. All board edits use KiCad's native pcbnew API."""
import math
import heapq
from array import array
import numpy as np
from PIL import Image, ImageDraw
import pcbnew as k

def xy(item):
    p=item.GetPosition() if hasattr(item,'GetPosition') else item
    return k.ToMM(p.x),k.ToMM(p.y)

def vec(p):
    return k.VECTOR2I(k.FromMM(p[0]),k.FromMM(p[1]))

def net(board,name):
    return board.FindNet(name).GetNetCode()

def pad(board,ref,num):
    return next(p for p in board.FindFootprintByReference(ref).Pads() if p.GetNumber()==str(num))

def line(board,name,points,width=.2,layer=k.F_Cu):
    out=[]
    nc=net(board,name) if isinstance(name,str) else name
    for a,b in zip(points,points[1:]):
        if math.dist(a,b)<1e-6: continue
        t=k.PCB_TRACK(board); t.SetStart(vec(a)); t.SetEnd(vec(b)); t.SetWidth(k.FromMM(width)); t.SetLayer(layer); t.SetNetCode(nc); board.Add(t); out.append(t)
    return out

def via(board,name,p,diameter=.6,drill=.3):
    v=k.PCB_VIA(board); v.SetPosition(vec(p)); v.SetViaType(k.VIATYPE_THROUGH); v.SetLayerPair(k.F_Cu,k.B_Cu); v.SetWidth(k.F_Cu,k.FromMM(diameter)); v.SetDrill(k.FromMM(drill)); v.SetNetCode(net(board,name) if isinstance(name,str) else name); board.Add(v); return v

def local_zone(board,name,points,layer=k.F_Cu,priority=20,clearance=.2):
    z=k.ZONE(board); z.SetLayer(layer); z.SetNetCode(net(board,name)); z.SetAssignedPriority(priority); z.SetLocalClearance(k.FromMM(clearance)); z.SetMinThickness(k.FromMM(.15)); z.SetPadConnection(k.ZONE_CONNECTION_FULL); z.SetThermalReliefGap(k.FromMM(.2)); z.SetThermalReliefSpokeWidth(k.FromMM(.25)); z.Outline().NewOutline()
    for p in points: z.Outline().Append(*[k.FromMM(v) for v in p])
    board.Add(z); return z

def keepout(board,name,points,layers):
    z=k.ZONE(board); s=k.LSET()
    for l in layers:s.AddLayer(l)
    z.SetLayerSet(s); z.SetIsRuleArea(True); z.SetZoneName(name); z.SetDoNotAllowTracks(True); z.SetDoNotAllowVias(True); z.SetDoNotAllowPads(True); z.SetDoNotAllowZoneFills(True); z.Outline().NewOutline()
    for p in points:z.Outline().Append(*[k.FromMM(v) for v in p])
    board.Add(z);return z

def _polygons(poly):
    for i in range(poly.OutlineCount()):
        o=poly.COutline(i)
        yield [xy(o.CPoint(j)) for j in range(o.PointCount())]

class GridRouter:
    """Octilinear path planning against native KiCad expanded copper outlines."""
    def __init__(self,board,layer=k.In2_Cu,grid=.1,width=.2,clearance=.2,bounds=(33,51.5,187,148.5)):
        self.b=board;self.layer=layer;self.grid=grid;self.width=width;self.clearance=clearance;self.bounds=bounds
        self.x0,self.y0,self.x1,self.y1=bounds
        self.nx=int(round((self.x1-self.x0)/grid))+1;self.ny=int(round((self.y1-self.y0)/grid))+1
    def pix(self,p):return round((p[0]-self.x0)/self.grid),round((p[1]-self.y0)/self.grid)
    def point(self,p):return round(p[0]*self.grid+self.x0,6),round(p[1]*self.grid+self.y0,6)
    def obstacles(self,nc):
        im=Image.new('1',(self.nx,self.ny),0); d=ImageDraw.Draw(im)
        clearance=k.FromMM(self.clearance+self.width/2+self.grid*.8)
        for f in self.b.GetFootprints():
            for p in f.Pads():
                if p.GetNetCode()==nc or not p.IsOnLayer(self.layer):continue
                poly=k.SHAPE_POLY_SET();p.TransformShapeToPolygon(poly,self.layer,clearance,k.FromMM(.005),k.ERROR_OUTSIDE)
                for points in _polygons(poly):
                    if len(points)>2:d.polygon([self.pix(x) for x in points],fill=1)
        for t in self.b.GetTracks():
            if t.GetNetCode()==nc or not t.IsOnLayer(self.layer):continue
            poly=k.SHAPE_POLY_SET();t.TransformShapeToPolygon(poly,self.layer,clearance,k.FromMM(.005),k.ERROR_OUTSIDE)
            for points in _polygons(poly):
                if len(points)>2:d.polygon([self.pix(x) for x in points],fill=1)
        for z in self.b.Zones():
            if not z.GetIsRuleArea() or not z.IsOnLayer(self.layer) or not z.GetDoNotAllowTracks():continue
            poly=k.SHAPE_POLY_SET(z.Outline());poly.Inflate(k.FromMM(self.width/2+self.grid*.8),k.CORNER_STRATEGY_ROUND_ALL_CORNERS,k.FromMM(.01))
            for points in _polygons(poly):
                if len(points)>2:d.polygon([self.pix(x) for x in points],fill=1)
        a=np.array(im,dtype=np.bool_); edge=int(math.ceil((self.width/2+.02)/self.grid))
        a[:edge,:]=True;a[-edge:,:]=True;a[:,:edge]=True;a[:,-edge:]=True
        return a
    def clear(self,a,p,q):
        p0=self.pix(p);p1=self.pix(q);n=max(abs(p1[0]-p0[0]),abs(p1[1]-p0[1]))*2+1
        xs=np.rint(np.linspace(p0[0],p1[0],n)).astype(int);ys=np.rint(np.linspace(p0[1],p1[1],n)).astype(int)
        return bool(np.all((xs>=0)&(xs<self.nx)&(ys>=0)&(ys<self.ny))) and not np.any(a[ys,xs])
    def candidates(self,p,q):
        dx=q[0]-p[0];dy=q[1]-p[1];sx=1 if dx>=0 else -1;sy=1 if dy>=0 else -1;m=min(abs(dx),abs(dy))
        if min(abs(dx),abs(dy))<1e-7 or abs(abs(dx)-abs(dy))<1e-7:return [[p,q]]
        return [[p,(p[0]+sx*m,p[1]+sy*m),q],[p,(q[0]-sx*m,q[1]-sy*m),q]]
    def route(self,name,start,end):
        nc=net(self.b,name);a=self.obstacles(nc)
        for candidate in self.candidates(start,end):
            if all(self.clear(a,p,q) for p,q in zip(candidate,candidate[1:])):
                line(self.b,nc,candidate,self.width,self.layer);return candidate
        sp=self.pix(start);ep=self.pix(end)
        if a[sp[1],sp[0]] or a[ep[1],ep[0]]:raise RuntimeError(f'Blocked routing terminal {name}: {start} {end}')
        n=self.nx*self.ny;dist=np.full(n,np.inf,dtype=np.float32);prev=np.full(n,-1,dtype=np.int32);closed=np.zeros(n,dtype=np.bool_)
        si=sp[1]*self.nx+sp[0];ei=ep[1]*self.nx+ep[0];dist[si]=0
        def heuristic(x,y):
            dx=abs(ep[0]-x);dy=abs(ep[1]-y);return max(dx,dy)+(math.sqrt(2)-1)*min(dx,dy)
        heap=[(heuristic(*sp),0,si)]; dirs=((1,0,1),(-1,0,1),(0,1,1),(0,-1,1),(1,1,math.sqrt(2)),(1,-1,math.sqrt(2)),(-1,1,math.sqrt(2)),(-1,-1,math.sqrt(2)))
        visited=0
        while heap:
            _,g,u=heapq.heappop(heap)
            if closed[u]:continue
            if u==ei:break
            closed[u]=True;visited+=1;x=u%self.nx;y=u//self.nx
            for dx,dy,cost in dirs:
                xx=x+dx;yy=y+dy
                if xx<0 or yy<0 or xx>=self.nx or yy>=self.ny or a[yy,xx]:continue
                if dx and dy and (a[y,xx] or a[yy,x]):continue
                v=yy*self.nx+xx
                if closed[v]:continue
                ng=g+cost
                if ng+.0001<float(dist[v]):
                    dist[v]=ng;prev[v]=u;heapq.heappush(heap,(ng+heuristic(xx,yy)*1.025,ng,v))
        if prev[ei]<0:raise RuntimeError(f'No {self.b.GetLayerName(self.layer)} route for {name}; examined {visited}')
        ids=[ei]
        while ids[-1]!=si:ids.append(int(prev[ids[-1]]))
        ids.reverse();pts=[self.point((v%self.nx,v//self.nx)) for v in ids];pts[0]=start;pts[-1]=end
        # Pull long octilinear shortcuts through the verified free raster.
        clean=[start];i=0
        while i<len(pts)-1:
            found=False
            for j in range(len(pts)-1,i,-1):
                for candidate in self.candidates(pts[i],pts[j]):
                    if all(self.clear(a,p,q) for p,q in zip(candidate,candidate[1:])):
                        clean.extend(candidate[1:]);i=j;found=True;break
                if found:break
            if not found:raise RuntimeError('Path simplification failed')
        line(self.b,nc,clean,self.width,self.layer)
        return clean

def render(board,path,bounds=(29,49,191,151),scale=12):
    x0,y0,x1,y1=bounds;im=Image.new('RGB',(int((x1-x0)*scale),int((y1-y0)*scale)),(12,26,34));d=ImageDraw.Draw(im)
    def p(v):return ((v[0]-x0)*scale,(v[1]-y0)*scale)
    for l,col in [(k.In2_Cu,(63,162,97)),(k.F_Cu,(210,68,59))]:
        for t in board.GetTracks():
            if isinstance(t,k.PCB_VIA):continue
            if t.GetLayer()!=l:continue
            d.line([p(xy(t.GetStart())),p(xy(t.GetEnd()))],fill=col,width=max(1,round(k.ToMM(t.GetWidth())*scale)))
    for f in board.GetFootprints():
        for pd in f.Pads():
            poly=k.SHAPE_POLY_SET();pd.TransformShapeToPolygon(poly,k.F_Cu,0,k.FromMM(.01),k.ERROR_OUTSIDE)
            for shape in _polygons(poly):
                if len(shape)>2:d.polygon([p(v) for v in shape],fill=(190,109,70),outline=(250,190,140))
        pp=xy(f);d.text(p((pp[0]-.3,pp[1]-1.2)),f.GetReference(),fill=(230,230,150),stroke_width=0)
    for t in board.GetTracks():
        if isinstance(t,k.PCB_VIA):
            x,y=p(xy(t));r=k.ToMM(t.GetWidth(k.F_Cu))*scale/2;d.ellipse((x-r,y-r,x+r,y+r),outline=(230,210,140),width=1)
    im.save(path)
