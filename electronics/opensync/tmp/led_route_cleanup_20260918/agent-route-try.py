from organize_leds import *
from route_led_backbone import GuardedRouter,crossover_route
from pcb_helpers import line
import sys,shutil

order=sys.argv[1] if len(sys.argv)>1 else 'asc'
b=k.LoadBoard(str(WORK/'stripped.kicad_pcb'))
routes=[]
escape_y={'LED_A':70.0,'LED_B':69.3,'LED_C':68.6,'LED_D':67.9,'LED_E':67.2,'LED_F':66.5,'LED_H':65.8,'LED_TRIG':65.1}
for name,a in SOURCES.items():
    if name in escape_y:
        end=(a[0],escape_y[name]);add_path(b,name,[a,end]);a=end
    routes.append((name,a,(DESTS[name],59.8)))
if order=='asc':routes.sort(key=lambda x:x[2][0])
elif order=='desc':routes.sort(key=lambda x:x[2][0],reverse=True)
elif order=='hard':routes.sort(key=lambda x:(x[0] not in ['LED_G','LED_GATE','LED_ACTIVE'],x[2][0]))
elif order=='west':routes.sort(key=lambda x:(x[0] in ['LED_G','LED_GATE','LED_ACTIVE'],-x[2][0]))
router=GuardedRouter(b,k.In2_Cu,grid=.1,width=.2,clearance=.20,bounds=(62,52,183,96))
result={}
removed=[]
for name,a,z in routes:
    print('Routing',name,flush=True)
    try:result[name]=router.route(net(name),a,z)
    except RuntimeError:result[name]=crossover_route(b,net(name),a,z)
    for v in list(b.GetTracks()):
        if not isinstance(v,k.PCB_VIA) or v.GetNetname()!=net(name):continue
        sh=v.GetEffectiveShape(k.In2_Cu)
        used=any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==net(name) and t.GetLayer()==k.In2_Cu and sh.Collide(t.GetEffectiveShape(k.In2_Cu),0) for t in b.GetTracks())
        if not used:b.Remove(v);removed.append(v)
out=WORK/('agent-'+order+'.kicad_pcb')
k.SaveBoard(str(out),b)
shutil.copy2(WORK/'before.kicad_pro',out.with_suffix('.kicad_pro'))
(WORK/('agent-'+order+'-paths.json')).write_text(json.dumps(result,indent=2))
render(b,out.with_suffix('.png'),bounds=(65,53,180,94),scale=20)
print('METRICS',order,'via',sum(isinstance(t,k.PCB_VIA) for t in b.GetTracks() if t.GetNetname() in NAMES),'tracks',sum(not isinstance(t,k.PCB_VIA) for t in b.GetTracks() if t.GetNetname() in NAMES),'length',sum(k.ToMM(t.GetLength()) for t in b.GetTracks() if t.GetNetname() in NAMES and not isinstance(t,k.PCB_VIA)),flush=True)
