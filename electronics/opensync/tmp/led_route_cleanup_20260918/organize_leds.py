"""LED-only PCB route candidates via native pcbnew; no footprint setters."""
from pathlib import Path
import sys, json, math
import pcbnew as k

ROOT=Path(__file__).resolve().parents[2]
WORK=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'tmp/routing_channel_review'))
sys.path.insert(0,str(ROOT/'tmp/routing_work'))
from route_channels import Collision
from pcb_helpers import render
from route_led_backbone import GuardedRouter

def xy(p): return (round(k.ToMM(p.x),6),round(k.ToMM(p.y),6))
def vec(p): return k.VECTOR2I(round(p[0]*1000000),round(p[1]*1000000))
def uid(t): return t.m_Uuid.AsString()
def net(name): return '/Indicator Panel/'+name
SOURCES={'LED_A':(75.8,81),'LED_B':(75,81),'LED_C':(74.2,81),
 'LED_D':(73.2,81),'LED_E':(72,82),'LED_F':(71.4,82.6),
 'LED_G':(81.5,88.8),'LED_H':(70.8,83.2),'LED_GATE':(79,89),
 'LED_TRIG':(70.2,83.8),'LED_ACTIVE':(81.1,90.6)}
DESTS=dict(zip(SOURCES,[176.8,168.8,160.8,152.8,144.8,136.8,128.8,120.8,112.8,105,96.8]))
NAMES={net(n) for n in SOURCES}

def prepare():
    b=k.LoadBoard(str(WORK/'source.kicad_pcb'))
    removed=[]
    for name,source in SOURCES.items():
        nn=net(name)
        front=[t for t in b.GetTracks() if not isinstance(t,k.PCB_VIA) and t.GetNetname()==nn and t.GetLayer()==k.F_Cu]
        connected=set()
        shapes=[p.GetEffectiveShape(k.F_Cu) for p in b.GetPads() if p.GetNetname()==nn and p.IsOnLayer(k.F_Cu)]
        while True:
            batch=[t for t in front if uid(t) not in connected and any(s.Collide(t.GetEffectiveShape(k.F_Cu),0) for s in shapes)]
            if not batch: break
            connected.update(uid(t) for t in batch)
            shapes.extend(t.GetEffectiveShape(k.F_Cu) for t in batch)
        for t in list(b.GetTracks()):
            if t.GetNetname()!=nn:continue
            v=isinstance(t,k.PCB_VIA)
            keep=(xy(t.GetPosition()) in (source,(DESTS[name],59.8))) if v else (uid(t) in connected)
            if not keep:b.Remove(t);removed.append(t)
    k.SaveBoard(str(WORK/'stripped.kicad_pcb'),b)
    return b,removed

def add_path(b,name,pts,layer=k.In2_Cu,width=.2):
    col=Collision(b,layer,net(name),width,.18)
    for a,z in zip(pts,pts[1:]):
        if not col.clear(a,z):
            hits=[{'net':t.GetNetname(),'at':xy(t.GetPosition())}
                for t,s in col.ob if s.Collide(k.SHAPE_SEGMENT(vec(a),vec(z),round(width*1000000)),180000)]
            raise RuntimeError(str((name,a,z,hits)))
    added=[]
    for a,z in zip(pts,pts[1:]):
        if math.dist(a,z)<1e-7:continue
        t=k.PCB_TRACK(b);t.SetStart(vec(a));t.SetEnd(vec(z));t.SetWidth(round(width*1000000));t.SetLayer(layer)
        t.SetNetCode(b.FindNet(net(name)).GetNetCode());b.Add(t);added.append(t)
    return added

if __name__=='__main__':
    b,removed=prepare()
    print('Removed obsolete LED backbone objects',len(removed),flush=True)
    lanes={'LED_A':74,'LED_B':73.4,'LED_C':72.8,'LED_D':72.2,'LED_E':71.6,'LED_F':71,'LED_H':69.8,'LED_TRIG':68.6}
    result={}
    for name,lane in lanes.items():
        a=SOURCES[name];z=(DESTS[name],59.8)
        pts=[a,(a[0],round(lane+2,6)),(round(a[0]+2,6),lane),(round(z[0]-(lane-z[1]),6),lane),z]
        try:
            add_path(b,name,pts);result[name]=pts
            print('Pass',name,pts,flush=True)
        except RuntimeError as e:print('Blocked',e,flush=True)
    k.SaveBoard(str(WORK/'west-draft.kicad_pcb'),b)
    render(b,WORK/'west-draft.png',bounds=(65,53,180,94),scale=20)
    (WORK/'west-paths.json').write_text(json.dumps(result,indent=2))
