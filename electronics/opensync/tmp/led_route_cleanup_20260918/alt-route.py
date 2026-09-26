from pathlib import Path
import sys, json, math
import pcbnew as k
from organize_leds import WORK, SOURCES, DESTS, net, xy, GuardedRouter
from pcb_helpers import render
from route_led_backbone import crossover_route

def run(order):
    b=k.LoadBoard(str(WORK/'stripped.kicad_pcb'))
    router=GuardedRouter(b,k.In2_Cu,grid=.1,width=.2,clearance=.20,bounds=(62,52,183,96))
    names=sorted(SOURCES,key=lambda n:DESTS[n],reverse=order=='desc')
    paths={}
    for n in names:
        start=(DESTS[n],59.8);end=SOURCES[n]
        try: paths[n]=router.route(net(n),start,end)
        except RuntimeError as ex:
            print('Fallback',n,str(ex),flush=True)
            paths[n]=crossover_route(b,net(n),start,end)
        print('Done',n,paths[n],flush=True)
    out=WORK/('alt-'+order+'.kicad_pcb')
    k.SaveBoard(str(out),b)
    render(b,out.with_suffix('.png'),bounds=(64,54,181,94),scale=24)
    out.with_suffix('.json').write_text(json.dumps(paths,indent=2))
    items=[t for t in b.GetTracks() if t.GetNetname().startswith('/Indicator Panel/LED_')]
    print('COMPLETE',str(out),'vias',sum(isinstance(t,k.PCB_VIA) for t in items),'segments',sum(not isinstance(t,k.PCB_VIA) for t in items),'length',sum(k.ToMM(t.GetLength()) for t in items if not isinstance(t,k.PCB_VIA)),flush=True)
run(sys.argv[1] if len(sys.argv)>1 else 'desc')
