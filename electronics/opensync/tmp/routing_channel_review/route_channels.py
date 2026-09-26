"""Add output channel copper using pcbnew; never alter footprint/pad geometry."""
import pcbnew as p
import math
import heapq
import itertools
from collections import defaultdict, Counter
from pathlib import Path

MM=p.FromMM
def vec(q): return p.VECTOR2I(MM(q[0]),MM(q[1]))
def xy(v): return (p.ToMM(v.x),p.ToMM(v.y))
def shifted(q,d): return (q[0]+d[0],q[1]+d[1])

class Copper:
    def __init__(self,b):
        self.b=b; self.added=[]
        self.nets={n.GetNetname():n.GetNetCode() for n in b.GetNetsByNetcode().values()}
    def track(self,a,z,net,width=.2,layer=p.F_Cu):
        if math.dist(a,z)<1e-6:return
        t=p.PCB_TRACK(self.b);t.SetStart(vec(a));t.SetEnd(vec(z));t.SetWidth(MM(width));t.SetLayer(layer);t.SetNetCode(self.nets[net]);self.b.Add(t);self.added.append(t)
        return t
    def path(self,pts,net,width=.2,layer=p.F_Cu):
        for a,z in zip(pts,pts[1:]):self.track(a,z,net,width,layer)
    def clone(self,t,delta,net=None):
        c=t.Duplicate();c.Move(vec(delta))
        if net:c.SetNetCode(self.nets[net])
        self.b.Add(c);self.added.append(c)
        return c

class Collision:
    """Exact KiCad copper shapes indexed by 2 mm tiles, ignoring pour polygons."""
    def __init__(self,b,layer,net,width=.4,clearance=.20):
        self.layer=layer;self.width=width;self.clearance=MM(clearance);self.tiles=defaultdict(list)
        self.ob=[]
        for item in itertools.chain(b.GetPads(),b.GetTracks()):
            if item.GetNetname()==net or not item.IsOnLayer(layer):continue
            sh=item.GetEffectiveShape(layer)
            bb=sh.BBox();a=xy(bb.GetPosition());z=xy(bb.GetEnd());margin=width/2+clearance+.02
            x0,x1=math.floor((a[0]-margin)/2),math.floor((z[0]+margin)/2)
            y0,y1=math.floor((a[1]-margin)/2),math.floor((z[1]+margin)/2)
            ix=len(self.ob);self.ob.append((item,sh))
            for x in range(x0,x1+1):
                for y in range(y0,y1+1):self.tiles[x,y].append(ix)
    def clear(self,a,z):
        cand=set()
        for x in range(math.floor(min(a[0],z[0])/2),math.floor(max(a[0],z[0])/2)+1):
            for y in range(math.floor(min(a[1],z[1])/2),math.floor(max(a[1],z[1])/2)+1):cand.update(self.tiles.get((x,y),[]))
        sh=p.SHAPE_SEGMENT(vec(a),vec(z),MM(self.width))
        return not any(self.ob[i][1].Collide(sh,self.clearance) for i in cand)

def bridge(c,start,end,net):
    col=Collision(c.b,p.In2_Cu,net)
    # Try short 45-degree routes before grid search.
    dx,dy=end[0]-start[0],end[1]-start[1]
    sx=1 if dx>=0 else -1;sy=1 if dy>=0 else -1;diag=min(abs(dx),abs(dy))
    candidates=[ [start,(start[0]+sx*diag,start[1]+sy*diag),end],
                 [start,(end[0]-sx*diag,end[1]-sy*diag),end] ]
    for pts in candidates:
        if all(col.clear(a,z) for a,z in zip(pts,pts[1:])):
            c.path(pts,net,.4,p.In2_Cu);return pts
    step=.2
    def grid(q):return (round((q[0]-.1)/step),round(q[1]/step))
    def point(q):return (q[0]*step+.1,q[1]*step)
    s,g=grid(start),grid(end)
    if not col.clear(start,point(s)) or not col.clear(point(g),end):raise RuntimeError('Grid endpoint obstructed '+net)
    q=[(0,0,s)];cost={s:0};parent={};seen=set();cache={}
    dirs=[(1,0),(-1,0),(0,1),(0,-1),(1,1),(1,-1),(-1,1),(-1,-1)]
    while q:
        _,gc,u=heapq.heappop(q)
        if u in seen:continue
        if u==g:break
        seen.add(u)
        if len(seen)>350000:raise RuntimeError('Routing search exhausted '+net)
        for d in dirs:
            v=(u[0]+d[0],u[1]+d[1]);vpt=point(v)
            if not (73<=vpt[0]<=185 and 76<=vpt[1]<=128):continue
            if v in seen:continue
            ng=gc+(1.41421356 if d[0] and d[1] else 1)
            if ng>=cost.get(v,float('inf')):continue
            edge=tuple(sorted((u,v)))
            valid=cache.get(edge)
            if valid is None:valid=col.clear(point(u),vpt);cache[edge]=valid
            if not valid:continue
            cost[v]=ng;parent[v]=u
            ddx,ddy=abs(v[0]-g[0]),abs(v[1]-g[1]);h=max(ddx,ddy)+.41421356*min(ddx,ddy)
            heapq.heappush(q,(ng+h,ng,v))
    else:raise RuntimeError('No In2 path '+net)
    nodes=[g]
    while nodes[-1]!=s:nodes.append(parent[nodes[-1]])
    nodes.reverse();simple=[nodes[0]]
    for i in range(1,len(nodes)-1):
        before=(nodes[i][0]-nodes[i-1][0],nodes[i][1]-nodes[i-1][1]);after=(nodes[i+1][0]-nodes[i][0],nodes[i+1][1]-nodes[i][1])
        if before!=after:simple.append(nodes[i])
    simple.append(nodes[-1]);pts=[start]+[point(n) for n in simple]+[end]
    c.path(pts,net,.4,p.In2_Cu)
    return pts

def apply(board):
    c=Copper(board);fps={f.GetReference():f for f in board.GetFootprints()}
    original=list(board.GetTracks());routes={}
    selector={n:(16*((n-1)%4),-21 if n<=4 else 0) for n in range(1,9)}
    driver={1:(-50,0),2:(-39,0),3:(-25,-.185407),4:(-14,-.185407),5:(0,0),6:(11,0),7:(25,0),8:(36,0)}
    # Clone the user's local VOUT routing and paired layer-change vias at each end.
    for n in (1,2,3,4,6,7,8):
        net=f'/Output Channel {n}/VOUT';ds,dd=selector[n],driver[n]
        for t in original:
            a,z=xy(t.GetStart()),xy(t.GetEnd())
            if t.GetNetname()=='/Output Channel 5/VOUT':
                if isinstance(t,p.PCB_VIA):c.clone(t,ds if a[1]<110 else dd,net)
                elif t.GetLayer()==p.F_Cu:c.clone(t,ds if max(a[1],z[1])<110 else dd,net)
                elif t.GetLayer()==p.In2_Cu and max(a[1],z[1])<=103.5:c.clone(t,ds,net)
                elif t.GetLayer()==p.In2_Cu and min(a[1],z[1])>=123.8:c.clone(t,dd,net)
            if t.GetNetname()=='+5V' and all(129<=q[0]<=133 and 92<=q[1]<=94 for q in (a,z)):
                if isinstance(t,p.PCB_VIA) or t.GetLayer()==p.F_Cu:c.clone(t,ds)
        # Preserve .6/.3 mm via geometry while giving copied hole pairs .3 mm
        # drill-edge clearance. The original CH5 pair spacing is untouched.
        moves=[(shifted((128.3,103.5),ds),(0,.1)),(shifted((130.8,123.8),dd),(-.1,0))]
        for item in c.added:
            if item.GetNetname()!=net:continue
            if isinstance(item,p.PCB_VIA):
                for at,delta in moves:
                    if math.dist(xy(item.GetPosition()),at)<1e-6:item.Move(vec(delta))
            else:
                for getter,setter in [(item.GetStart,item.SetStart),(item.GetEnd,item.SetEnd)]:
                    for at,delta in moves:
                        if math.dist(xy(getter()),at)<1e-6:setter(vec(shifted(at,delta)))
        c.path([shifted((128.3,103.0),ds),shifted((128.3,103.6),ds)],net,.3)
    # Explicit short capacitor-to-supply-pin paths and pull-up connections.
    for n in range(1,9):
        ds,dd=selector[n],driver[n]
        c.path([shifted(q,ds) for q in [(127,92.625),(128.435,92.625),(128.56,92.75)]],'+3V3',.3)
        c.path([shifted(q,ds) for q in [(128.56,92.25),(128.56,92.75)]],'+3V3',.2)
        c.path([shifted(q,dd) for q in [(130,120.285),(128.75,120.285),(128.750001,120.6284)]],'+3V3',.3)
        c.path([shifted(q,dd) for q in [(126.1,122.69),(126.1,122.0),(128.75,122.0),(128.750001,120.6284)]],'+3V3',.2)
        c.path([shifted(q,dd) for q in [(126.1,123.71),(126.911599,123.71),(127.249999,123.3716)]],f'Net-(U{n*2}-DIR)',.2)
    # Extend each tiny capacitor GND zone around its existing pad so thermal spokes fit.
    zones_changed=[]
    for z in board.Zones():
        if z.GetNetname()!='GND' or z.GetLayer()!=p.F_Cu:continue
        bb=z.GetBoundingBox();a=xy(bb.GetPosition());e=xy(bb.GetEnd())
        for n in range(1,9):
            cap=fps[f'C{5*n+16}'];pad=next(q for q in cap.Pads() if q.GetNumber()=='2');q=xy(pad.GetPosition())
            if 0<a[0]-q[0]<1.0 and abs((a[1]+e[1])/2-q[1])<.1 and e[0]-a[0]<3:
                poly=z.Outline()
                while poly.OutlineCount():poly.RemoveOutline(0)
                poly.NewOutline()
                for x,y in [(q[0]-1.8,q[1]-1.6),(e[0],q[1]-1.6),(e[0],q[1]+1.6),(q[0]-1.8,q[1]+1.6)]:poly.Append(MM(x),MM(y))
                zones_changed.append(z.m_Uuid.AsString());break
    for n in (1,2,3,4,6,7,8):
        net=f'/Output Channel {n}/VOUT'
        start=shifted((128.3,103.0),selector[n]);end=shifted((131.3,123.8),driver[n])
        if n<=4:
            # Upper selectors feed leftward above the lower selector row. This
            # avoids enclosing lower-channel feeds behind crossing power traces.
            col=Collision(board,p.In2_Cu,net)
            offsets=[10,10.2,10.4,10.6,10.8] if n==1 else [1,2,3,4,5,6]
            mid=next(((end[0]+dx,84.8+n-1) for dx in offsets if col.clear((end[0]+dx,84.8+n-1),(end[0]+dx,84.8+n-1))),None)
            if mid is None:raise RuntimeError('No upper-channel bridge waypoint '+net)
            routes[n]=bridge(c,start,mid,net)+bridge(c,mid,end,net)[1:]
        else:routes[n]=bridge(c,start,end,net)
        print('Routed',net,len(routes[n]),'vertices',flush=True)
    return {'added':len(c.added),'zones_changed':zones_changed,'bridges':routes}

if __name__=='__main__':
    import shutil,json
    root=Path(__file__).resolve().parents[2];out=Path(__file__).resolve().parent/'channel-draft.kicad_pcb'
    b=p.LoadBoard(str(root/'opensync.kicad_pcb'));print(json.dumps(apply(b),indent=2))
    p.ZONE_FILLER(b).Fill(b.Zones());p.SaveBoard(str(out),b)
    shutil.copy2(root/'opensync.kicad_pro',out.with_suffix('.kicad_pro'))
