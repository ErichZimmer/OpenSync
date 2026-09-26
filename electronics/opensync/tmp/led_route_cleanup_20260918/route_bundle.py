"""Keep the LED trunk organized; route only its source-side links."""
from organize_leds import *
from route_led_backbone import crossover_route
import shutil

b=k.LoadBoard(str(WORK/'stripped.kicad_pcb'))
order=list(SOURCES)
lanes={name:round(68.8-.6*i,1) for i,name in enumerate(order)}
lanes['LED_ACTIVE']=62.6
result={}
for name,y in lanes.items():
    z=(DESTS[name],59.8)
    pts=[(94,y),(round(z[0]-(y-59.8),6),y),z]
    add_path(b,name,pts)
    result[name]={'trunk':pts}
print('All parallel trunks clear',flush=True)
router=GuardedRouter(b,k.In2_Cu,grid=.1,width=.2,clearance=.20,bounds=(65,61,95,96))
for name in order:
    print('Routing source',name,flush=True)
    try:result[name]['source']=router.route(net(name),SOURCES[name],(94,lanes[name]))
    except RuntimeError as e:
        print('Inner-layer path unavailable:',e,flush=True)
        result[name]['source']=crossover_route(b,net(name),SOURCES[name],(94,lanes[name]))
    print(result[name]['source'],flush=True)
k.SaveBoard(str(WORK/'bundle-candidate.kicad_pcb'),b)
shutil.copy2(WORK/'before.kicad_pro',WORK/'bundle-candidate.kicad_pro')
(WORK/'bundle-paths.json').write_text(json.dumps(result,indent=2))
render(b,WORK/'bundle-candidate.png',bounds=(65,53,180,94),scale=20)
