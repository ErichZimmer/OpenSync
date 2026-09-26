import sys,json,shutil
from pathlib import Path
import pcbnew as k
ROOT=Path(__file__).resolve().parents[2]
for folder in ('routing_power_review','routing_channel_review','routing_digital_review'):
    sys.path.insert(0,str(ROOT/'tmp'/folder))
import route_power,route_channels,route_digital,route_remaining,fix_ch5_vias,cleanup_routes
import finish_vin,bridge_5v_plane
from route_led_backbone import route_led_backbone
from pcb_helpers import render

b=k.LoadBoard(str(ROOT/'opensync.kicad_pcb'))
routes=route_remaining.apply(b,backbone=False)
print('POWER',route_power.apply(b),flush=True)
print('CHANNELS',route_channels.apply(b),flush=True)
print('DIGITAL',route_digital.apply(b),flush=True)
print('CH5 via spacing',fix_ch5_vias.apply(b),flush=True)
print('Removed dangling stubs',cleanup_routes.apply(b),flush=True)
print('LED terminals',routes,flush=True)
if '--led' in sys.argv:
    print('LED backbone',route_led_backbone(b,routes),flush=True)
    print('C1 solid local supply',finish_vin.apply(b),flush=True)
    if '--5v-bridge' in sys.argv:print('5V bridge',bridge_5v_plane.apply(b),flush=True)
out=ROOT/'tmp'/'routing_work'/'integrated.kicad_pcb'
expected_nets={t.m_Uuid.AsString():t.GetNetname() for t in b.GetTracks()}
k.ZONE_FILLER(b).Fill(b.Zones())
changed_nets=[(t.m_Uuid.AsString(),expected_nets[t.m_Uuid.AsString()],t.GetNetname()) for t in b.GetTracks() if expected_nets[t.m_Uuid.AsString()]!=t.GetNetname()]
k.SaveBoard(str(out),b)
assert not changed_nets,repr(changed_nets)
shutil.copy2(ROOT/'opensync.kicad_pro',out.with_suffix('.kicad_pro'))
render(b,ROOT/'tmp'/'routing_work'/'integrated.png')
print(out,flush=True)
