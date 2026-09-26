"""Route LED GPIO breakouts on In2.Cu after terminal fanouts exist."""
import sys
from pathlib import Path
import pcbnew as k

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tmp'/'routing_work'))
from pcb_helpers import xy,pad,GridRouter,render,line,via,net
import numpy as np
import math,heapq
from PIL import Image,ImageDraw

class GuardedRouter(GridRouter):
    """Planning-only exclusion keeps LED copper away from the unshielded LX node."""
    def obstacles(self,nc):
        a=super().obstacles(nc);im=Image.fromarray(a);draw=ImageDraw.Draw(im)
        for z in self.b.Zones():
            if z.GetZoneName()!='RP2354 LX L2 copper keepout':continue
            poly=k.SHAPE_POLY_SET(z.Outline())
            poly.Inflate(k.FromMM(self.clearance+self.width/2+self.grid*.8),k.CORNER_STRATEGY_ROUND_ALL_CORNERS,k.FromMM(.01))
            for i in range(poly.OutlineCount()):
                outline=poly.COutline(i)
                pts=[self.pix(xy(outline.CPoint(j))) for j in range(outline.PointCount())]
                draw.polygon(pts,fill=1)
        return np.array(im,dtype=np.bool_)

def crossover_route(b,nn,start,end):
    """Two signal layers only; costly vias encourage brief top-layer crossovers."""
    layers=[k.In2_Cu,k.F_Cu];bounds=(62,52,183,96);grid=.1
    rs=[GuardedRouter(b,l,grid=grid,width=.2,clearance=.20,bounds=bounds) for l in layers]
    nc=net(b,nn);aa=[r.obstacles(nc) for r in rs];r=rs[0]
    vv=[GuardedRouter(b,l,grid=grid,width=.6,clearance=.20,bounds=bounds).obstacles(nc) for l in layers]
    viaok=~(vv[0]|vv[1]);nx,ny=r.nx,r.ny;n=nx*ny
    sp,ep=r.pix(start),r.pix(end);si=sp[1]*nx+sp[0];ei=ep[1]*nx+ep[0]
    if aa[0][sp[1],sp[0]] or aa[0][ep[1],ep[0]]:raise RuntimeError('Blocked crossover terminal '+nn)
    existing={r.pix(xy(t)) for t in b.GetTracks() if isinstance(t,k.PCB_VIA) and t.GetNetCode()==nc}
    for x,y in existing:
        for dx in range(-6,7):
            for dy in range(-6,7):
                if dx*dx+dy*dy<36 and 0<=x+dx<nx and 0<=y+dy<ny:viaok[y+dy,x+dx]=False
    for x,y in existing:
        if 0<=x<nx and 0<=y<ny:viaok[y,x]=True
    # Existing terminal vias may be used to start/end on F.Cu directly when
    # another route cages the inner-layer escape. Redundant ones are removed
    # after routing if no In2 connection actually uses them.
    dist=np.full(n*2,np.inf,dtype=np.float32);prev=np.full(n*2,-1,dtype=np.int32);done=np.zeros(n*2,dtype=np.bool_)
    def h(x,y):
        dx,dy=abs(x-ep[0]),abs(y-ep[1]);return max(dx,dy)+.41421356*min(dx,dy)
    dist[si]=0;heap=[(h(*sp),0,si)];count=0
    dirs=[(1,0,1),(-1,0,1),(0,1,1),(0,-1,1),(1,1,1.41421356),(1,-1,1.41421356),(-1,1,1.41421356),(-1,-1,1.41421356)]
    while heap:
        _,gc,u=heapq.heappop(heap)
        if done[u]:continue
        if u==ei:break
        done[u]=True;count+=1
        if count>800000:raise RuntimeError('Crossover search exhausted '+nn)
        l=u//n;j=u%n;x,y=j%nx,j//nx;a=aa[l]
        for dx,dy,d in dirs:
            xx,yy=x+dx,y+dy
            if not(0<=xx<nx and 0<=yy<ny) or a[yy,xx]:continue
            if dx and dy and (a[y,xx] or a[yy,x]):continue
            v=l*n+yy*nx+xx;ng=gc+d*(1.5 if l else 1)
            if not done[v] and ng+.0001<dist[v]:
                dist[v]=ng;prev[v]=u;heapq.heappush(heap,(ng+h(xx,yy)*1.01,ng,v))
        if viaok[y,x] and not aa[1-l][y,x]:
            v=(1-l)*n+j;ng=gc+(0.1 if (x,y) in existing else 35)
            if not done[v] and ng+.0001<dist[v]:dist[v]=ng;prev[v]=u;heapq.heappush(heap,(ng+h(x,y),ng,v))
    else:raise RuntimeError('No crossover route '+nn)
    ids=[ei]
    while ids[-1]!=si:ids.append(int(prev[ids[-1]]))
    ids.reverse();chunks=[];curr=[];cl=0;addedvia=[]
    for i,u in enumerate(ids):
        l=u//n;j=u%n;pt=r.point((j%nx,j//nx))
        if i==0:pt=start
        if i==len(ids)-1:pt=end
        if l!=cl:
            chunks.append((cl,curr));curr=[pt];cl=l
            if (j%nx,j//nx) not in existing:
                via(b,nn,pt);addedvia.append(pt);existing.add((j%nx,j//nx))
        else:curr.append(pt)
    chunks.append((cl,curr));out=[]
    for l,pts in chunks:
        if len(pts)<2:continue
        clean=[pts[0]];i=0
        while i<len(pts)-1:
            for j in range(len(pts)-1,i,-1):
                choices=[c for c in rs[l].candidates(pts[i],pts[j]) if all(rs[l].clear(aa[l],p,q) for p,q in zip(c,c[1:]))]
                if choices:clean+=choices[0][1:];i=j;break
            else:raise RuntimeError('Could not simplify crossover '+nn)
        line(b,nn,clean,.2,layers[l]);out.append((b.GetLayerName(layers[l]),clean))
    print('Crossover',nn,'new vias',len(addedvia),'visited',count,flush=True)
    return out

def terminals(b):
    names=['LED_A','LED_B','LED_C','LED_D','LED_E','LED_F','LED_G','LED_H','LED_GATE','LED_TRIG','LED_ACTIVE']
    out=[]
    for name in names:
        nn='/Indicator Panel/'+name
        vs=[t for t in b.GetTracks() if isinstance(t,k.PCB_VIA) and t.GetNetname()==nn]
        out.append((nn,xy(next(v for v in vs if xy(v)[1]>70)),xy(next(v for v in vs if xy(v)[1]<65))))
    nn='/Indicator Panel/BTN_START';source=next(t for t in b.GetTracks() if isinstance(t,k.PCB_VIA) and t.GetNetname()==nn)
    out.append((nn,xy(source),xy(pad(b,'SW3',2))))
    return out

def route_led_backbone(b,routes=None,order='ascending'):
    routes=terminals(b) if routes is None else routes
    if not any(n.endswith('BTN_START') for n,a,z in routes):
        routes=list(routes)+[terminals(b)[-1]]
    # Give every tightly grouped west-side source a verified inner-layer escape
    # before any long backbone can surround an unrouted source via.
    from route_channels import Collision
    escape_y={'LED_A':70.0,'LED_B':69.3,'LED_C':68.6,'LED_D':67.9,'LED_E':67.2,'LED_F':66.5,'LED_H':65.8,'LED_TRIG':65.1}
    escaped=[]
    for nn,a,z in routes:
        name=nn.rsplit('/',1)[-1]
        if name in escape_y:
            end=(a[0],escape_y[name])
            if not Collision(b,k.In2_Cu,nn,width=.2,clearance=.18).clear(a,end):raise RuntimeError('GPIO escape blocked '+nn)
            line(b,nn,[a,end],.2,k.In2_Cu);a=end
        escaped.append((nn,a,z))
    routes=escaped
    if order=='ascending':routes=sorted(routes,key=lambda r:r[2][0])
    elif order=='descending':routes=sorted(routes,key=lambda r:r[2][0],reverse=True)
    router=GuardedRouter(b,k.In2_Cu,grid=.1,width=.2,clearance=.20,bounds=(62,52,183,96))
    result={}
    for nn,a,z in routes:
        try:pts=router.route(nn,a,z);result[nn]=pts
        except RuntimeError:
            print('Adding short crossover for',nn,flush=True)
            result[nn]=crossover_route(b,nn,a,z)
        for v in list(b.GetTracks()):
            if not isinstance(v,k.PCB_VIA) or v.GetNetname()!=nn or min(math.dist(xy(v),a),math.dist(xy(v),z))>1e-5:continue
            shape=v.GetEffectiveShape(k.In2_Cu)
            used=any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==nn and t.GetLayer()==k.In2_Cu and shape.Collide(t.GetEffectiveShape(k.In2_Cu),0) for t in b.GetTracks())
            if not used:
                print('Removed redundant terminal via',nn,xy(v),flush=True);b.Remove(v)
        print('Routed LED',nn,flush=True)
    return result

if __name__=='__main__':
    import json,shutil
    board=k.LoadBoard(str(ROOT/'tmp'/'routing_work'/'integrated.kicad_pcb'))
    result=route_led_backbone(board,order=sys.argv[1] if len(sys.argv)>1 else 'ascending')
    out=Path(__file__).resolve().parent/'led-draft.kicad_pcb'
    k.ZONE_FILLER(board).Fill(board.Zones());k.SaveBoard(str(out),board)
    shutil.copy2(ROOT/'opensync.kicad_pro',out.with_suffix('.kicad_pro'))
    render(board,out.with_suffix('.png'))
    print(json.dumps(result,indent=2))
