"""Read-only LED escape planning: never edits or saves a PCB."""
import sys, math, itertools, heapq
from collections import defaultdict
import pcbnew as k
sys.path.insert(0, 'tmp/routing_channel_review')
from route_channels import vec, xy

b=k.LoadBoard('tmp/led_route_cleanup_20260918/before.kicad_pcb')
src={'A':(75.8,81),'B':(75,81),'C':(74.2,81),'D':(73.2,81),'E':(72,82),'F':(71.4,82.6),'G':(81.5,88.8),'H':(70.8,83.2),'GATE':(79,89),'TRIG':(70.2,83.8),'ACTIVE':(81.1,90.6)}
all_src=list(src.values())
ob=[]
for t in itertools.chain(b.GetPads(),b.GetTracks()):
    if not t.IsOnLayer(k.In2_Cu):continue
    n=t.GetNetname()
    if n.startswith('/Indicator Panel/LED_'):
        if not isinstance(t,k.PCB_VIA):continue
        q=xy(t.GetPosition())
        if not (any(math.dist(q,s)<.01 for s in all_src) or abs(q[1]-59.8)<.01):continue
    ob.append((n,t.GetEffectiveShape(k.In2_Cu),t))

class Col:
    def __init__(self,net,extra=[]):
        self.obs=[(n,s,t) for n,s,t in ob+extra if n!=net]
        self.tiles=defaultdict(list)
        for idx,(_,s,_) in enumerate(self.obs):
            bb=s.BBox();a=xy(bb.GetPosition());z=xy(bb.GetEnd());r=.3
            for x in range(math.floor((a[0]-r)/2),math.floor((z[0]+r)/2)+1):
                for y in range(math.floor((a[1]-r)/2),math.floor((z[1]+r)/2)+1):self.tiles[x,y].append(idx)
    def hits(self,a,z):
        cand=set()
        for x in range(math.floor(min(a[0],z[0])/2),math.floor(max(a[0],z[0])/2)+1):
            for y in range(math.floor(min(a[1],z[1])/2),math.floor(max(a[1],z[1])/2)+1):cand.update(self.tiles[x,y])
        sh=k.SHAPE_SEGMENT(vec(a),vec(z),k.FromMM(.2))
        return [(self.obs[i][0],xy(self.obs[i][1].BBox().GetPosition())) for i in cand if self.obs[i][1].Collide(sh,k.FromMM(.18))]
    def clear(self,a,z):return not self.hits(a,z)

def route(col,start,end,bounds=(67,86,77,93),step=.1):
    point=lambda q:(round(q[0]*step,5),round(q[1]*step,5))
    grid=lambda q:(round(q[0]/step),round(q[1]/step))
    s,g=grid(start),grid(end);cost={s:0};parent={};seen=set();q=[(0,0,s)];cache={}
    dirs=[(1,0),(-1,0),(0,1),(0,-1),(1,1),(1,-1),(-1,1),(-1,-1)]
    while q:
        _,gc,u=heapq.heappop(q)
        if u in seen:continue
        if u==g:break
        seen.add(u)
        for d in dirs:
            v=(u[0]+d[0],u[1]+d[1]);pt=point(v)
            if not(bounds[0]<=pt[0]<=bounds[1] and bounds[2]<=pt[1]<=bounds[3]) or v in seen:continue
            ng=gc+(math.sqrt(2) if all(d) else 1)
            if ng>=cost.get(v,1e20):continue
            edge=tuple(sorted((u,v)))
            if edge not in cache:cache[edge]=col.clear(point(u),pt)
            if not cache[edge]:continue
            cost[v]=ng;parent[v]=u
            dx,dy=abs(v[0]-g[0]),abs(v[1]-g[1]);h=max(dx,dy)+(math.sqrt(2)-1)*min(dx,dy)
            heapq.heappush(q,(ng+h,ng,v))
    else:return None
    ns=[g]
    while ns[-1]!=s:ns.append(parent[ns[-1]])
    ns.reverse();simple=[ns[0]]
    for i in range(1,len(ns)-1):
        if (ns[i][0]-ns[i-1][0],ns[i][1]-ns[i-1][1]) != (ns[i+1][0]-ns[i][0],ns[i+1][1]-ns[i][1]):simple.append(ns[i])
    simple.append(ns[-1]);return [point(n) for n in simple]

if __name__=='__main__':
    extra=[]
    for name,end in [('G',(68,79)),('GATE',(68.6,79)),('ACTIVE',(69.2,79))]:
        n='/Indicator Panel/LED_'+name;col=Col(n,extra)
        pts=route(col,src[name],end)
        print(name,pts,flush=True)
        if pts:
            for a,z in zip(pts,pts[1:]):extra.append((n,k.SHAPE_SEGMENT(vec(a),vec(z),k.FromMM(.2)),None))
