from organize_leds import *
from route_led_backbone import GuardedRouter,crossover_route
from pcb_helpers import via
import shutil
b=k.LoadBoard(str(WORK/'agent-manual-west.kicad_pcb'))
removed=[]
for t in list(b.GetTracks()):
    if t.GetNetname() in {net('LED_A'),net('LED_B'),net('LED_C')} and not isinstance(t,k.PCB_VIA) and t.GetLayer()==k.In2_Cu:b.Remove(t);removed.append(t)
for name,v in {'LED_A':76.5,'LED_B':75.9,'LED_C':75.3}.items():
    add_path(b,name,[SOURCES[name],(SOURCES[name][0],v+1),(SOURCES[name][0]+1,v),(94,v),(102,v-8),(DESTS[name]-(v-8-59.8),v-8),(DESTS[name],59.8)])
trunks={'LED_G':[(92.6,70.5),(94,70.5),(102,62.5),(126.1,62.5),(128.8,59.8)],'LED_GATE':[(92.6,69.3),(94,69.3),(102,61.3),(111.3,61.3),(112.8,59.8)],'LED_ACTIVE':[(92.6,66.9),(94,66.9),(96.8,64.1),(96.8,59.8)]}
for name,pts in trunks.items():
    add_path(b,name,pts)
    at=pts[0]
    for lay in [k.F_Cu,k.In1_Cu,k.In2_Cu,k.B_Cu]:
        assert Collision(b,lay,net(name),.6,.18).clear(at,at),(name,'via collision',lay)
    via(b,net(name),at)
result={}
for name in ['LED_G','LED_GATE','LED_ACTIVE']:
    a=SOURCES[name];z=trunks[name][0];print('Routing',name,flush=True)
    result[name]=crossover_route(b,net(name),a,z)
    print(result[name],flush=True)
    for v in list(b.GetTracks()):
        if not isinstance(v,k.PCB_VIA) or v.GetNetname()!=net(name):continue
        sh=v.GetEffectiveShape(k.In2_Cu)
        used=any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==net(name) and t.GetLayer()==k.In2_Cu and sh.Collide(t.GetEffectiveShape(k.In2_Cu),0) for t in b.GetTracks())
        if not used:b.Remove(v);removed.append(v)
out=WORK/'agent-entry-vias.kicad_pcb'
k.SaveBoard(str(out),b);shutil.copy2(WORK/'before.kicad_pro',out.with_suffix('.kicad_pro'))
out.with_suffix('.json').write_text(json.dumps(result,indent=2))
render(b,out.with_suffix('.png'),bounds=(65,53,180,94),scale=20)
print('METRICS via',sum(isinstance(t,k.PCB_VIA) for t in b.GetTracks() if t.GetNetname() in NAMES),'tracks',sum(not isinstance(t,k.PCB_VIA) for t in b.GetTracks() if t.GetNetname() in NAMES),'length',sum(k.ToMM(t.GetLength()) for t in b.GetTracks() if t.GetNetname() in NAMES and not isinstance(t,k.PCB_VIA)),flush=True)
