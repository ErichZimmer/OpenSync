"""Ordered west-source bundle with short layer crossings for east sources."""
from organize_leds import *
from route_led_backbone import crossover_route
import shutil

b=k.LoadBoard(str(WORK/'stripped.kicad_pcb'))
west=['LED_A','LED_B','LED_C','LED_D','LED_E','LED_F','LED_H','LED_TRIG']
result={}
for i,name in enumerate(west):
    base=round(63.2-.6*i,1)
    delta=4.2 if i<6 else (3.6 if name=='LED_H' else 3)
    lane=round(base+delta,1)
    a=SOURCES[name];z=(DESTS[name],59.8)
    pts=[a,(a[0],round(base+2,1)),(round(a[0]+2,1),base),(94,base),
        (round(94+delta,1),lane),(round(z[0]-(lane-59.8),1),lane),z]
    add_path(b,name,pts);result[name]=pts
    print('Ordered',name,pts,flush=True)
for name,lane in [('LED_G',63.8),('LED_GATE',62.6)]:
    z=(DESTS[name],59.8);a=(106 if name=='LED_G' else 104,lane)
    pts=[a,(round(z[0]-(lane-59.8),1),lane),z]
    add_path(b,name,pts)
    result[name]={'trunk':pts,'source':crossover_route(b,net(name),SOURCES[name],a)}
    print('Special',name,result[name],flush=True)
name='LED_ACTIVE';a=(94,58);z=(96.8,59.8)
pts=[a,(95.8,59.8),z]
add_path(b,name,pts)
result[name]={'trunk':pts,'source':crossover_route(b,net(name),SOURCES[name],a)}
print('Special',name,result[name],flush=True)
# Remove any original source via no longer serving an inner-layer trace.
removed=[]
for name,at in SOURCES.items():
    for v in list(b.GetTracks()):
        if not isinstance(v,k.PCB_VIA) or v.GetNetname()!=net(name) or xy(v.GetPosition())!=at:continue
        sh=v.GetEffectiveShape(k.In2_Cu)
        used=any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==net(name) and t.GetLayer()==k.In2_Cu
            and sh.Collide(t.GetEffectiveShape(k.In2_Cu),0) for t in b.GetTracks())
        if not used:b.Remove(v);removed.append(v)
k.SaveBoard(str(WORK/'ordered-candidate.kicad_pcb'),b)
shutil.copy2(WORK/'before.kicad_pro',WORK/'ordered-candidate.kicad_pro')
(WORK/'ordered-paths.json').write_text(json.dumps(result,indent=2))
render(b,WORK/'ordered-candidate.png',bounds=(65,53,180,94),scale=20)
