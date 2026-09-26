from organize_leds import *
from route_led_backbone import GuardedRouter,crossover_route
import shutil
b=k.LoadBoard(str(WORK/'agent-manual-west.kicad_pcb'))
trunks={'LED_G':[(94,70.5),(102,62.5),(126.1,62.5),(128.8,59.8)],'LED_GATE':[(94,69.3),(102,61.3),(111.3,61.3),(112.8,59.8)],'LED_ACTIVE':[(94,66.9),(96.8,64.1),(96.8,59.8)]}
for name,pts in trunks.items():
    pts.insert(0,(93,pts[0][1]))
    add_path(b,name,pts)
router=GuardedRouter(b,k.In2_Cu,grid=.1,width=.2,clearance=.20,bounds=(65,60,95,95))
result={}
removed=[]
for name in sys.argv[1:] or ['LED_G','LED_GATE','LED_ACTIVE']:
    a=SOURCES[name];z=trunks[name][0];print('Routing',name,flush=True)
    try:result[name]=router.route(net(name),a,z)
    except RuntimeError:result[name]=crossover_route(b,net(name),a,z)
    for v in list(b.GetTracks()):
        if not isinstance(v,k.PCB_VIA) or v.GetNetname()!=net(name):continue
        sh=v.GetEffectiveShape(k.In2_Cu)
        used=any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==net(name) and t.GetLayer()==k.In2_Cu and sh.Collide(t.GetEffectiveShape(k.In2_Cu),0) for t in b.GetTracks())
        if not used:b.Remove(v);removed.append(v)
    print(result[name],flush=True)
tag='-'.join(sys.argv[1:]) if len(sys.argv)>1 else 'default'
out=WORK/('agent-manual-'+tag+'.kicad_pcb')
k.SaveBoard(str(out),b);shutil.copy2(WORK/'before.kicad_pro',out.with_suffix('.kicad_pro'))
out.with_suffix('.json').write_text(json.dumps(result,indent=2))
render(b,out.with_suffix('.png'),bounds=(65,53,180,94),scale=20)
print('METRICS',tag,'via',sum(isinstance(t,k.PCB_VIA) for t in b.GetTracks() if t.GetNetname() in NAMES),'tracks',sum(not isinstance(t,k.PCB_VIA) for t in b.GetTracks() if t.GetNetname() in NAMES),'length',sum(k.ToMM(t.GetLength()) for t in b.GetTracks() if t.GetNetname() in NAMES and not isinstance(t,k.PCB_VIA)),flush=True)
