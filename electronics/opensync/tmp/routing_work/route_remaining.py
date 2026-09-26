import sys
import math
from pathlib import Path
import pcbnew as k
from pcb_helpers import xy,vec,net,pad,line,via,keepout,local_zone,GridRouter,render

ROOT=Path(__file__).resolve().parents[2]

def move_via(b,uid,new):
    v=next(t for t in b.GetTracks() if t.m_Uuid.AsString()==uid);old=v.GetPosition();nc=v.GetNetCode()
    for t in b.GetTracks():
        if isinstance(t,k.PCB_VIA) or t.GetNetCode()!=nc:continue
        if t.GetStart()==old:t.SetStart(vec(new))
        if t.GetEnd()==old:t.SetEnd(vec(new))
    v.SetPosition(vec(new))

def apply(b,backbone=True):
    # Entire footprint rotation, retaining all pad geometry and net assignments.
    f=b.FindFootprintByReference('R65')
    if abs(f.GetOrientationDegrees()+90)<.001:
        label=f.Reference();lp=k.VECTOR2I(label.GetPosition());la=label.GetTextAngle()
        f.Rotate(f.GetPosition(),k.EDA_ANGLE(180,k.DEGREES_T))
        label.SetPosition(lp);label.SetTextAngle(la)
    for uid,new in [('14e32622-cf42-428e-9cbc-5f9e7405e1c6',(71.4,82.6)),('251f4181-0354-4104-b365-da5ca2c09fb1',(70.8,83.2)),('f1a327d5-a2fc-4c4d-bbb6-b910e64c7072',(70.2,83.8)),('5d2bb7eb-599b-45f6-ad60-a9e6b7dbeee6',(92,87.6))]:move_via(b,uid,new)
    keepout(b,'Left enclosure guide - 3mm',[(30,50),(33,50),(33,150),(30,150)],[k.F_Cu,k.In1_Cu,k.In2_Cu,k.B_Cu])
    keepout(b,'Right enclosure guide - 3mm',[(187,50),(190,50),(190,150),(187,150)],[k.F_Cu,k.In1_Cu,k.In2_Cu,k.B_Cu])
    # Keep clearances above the existing 0.18mm rule while allowing supply necks.
    for z in b.Zones():
        if z.GetNetname()=='+3V3' and z.GetAssignedPriority()==0:
            z.SetLocalClearance(k.FromMM(.2))
            z.SetThermalReliefGap(k.FromMM(.2))
            z.SetThermalReliefSpokeWidth(k.FromMM(.25))
        if not z.GetIsRuleArea():z.SetIslandRemovalMode(k.ISLAND_REMOVAL_MODE_ALWAYS)
    # Local supply-to-bypass traces for input Schmitt buffers and inverters.
    for c,u in [('C63','U33'),('C64','U34'),('C65','U35'),('C66','U36')]:
        p=xy(pad(b,c,1));q=xy(pad(b,u,5));d=q[1]-p[1]
        line(b,'+3V3',[p,(q[0]-abs(d),p[1]),q],.2)
    for u in ['U34','U36']:
        p=xy(pad(b,u,2));q=xy(pad(b,u,5))
        line(b,'+3V3',[p,(p[0],q[1]+abs(p[0]-q[0])),q],.2)
    # Complete reset-button contact copper and nearby resistor connection.
    p=pad(b,'R3',1);q=pad(b,'SW2',1)
    line(b,p.GetNetname(),[xy(p),(37.775,138.2),xy(q)],.2)
    swpads=[p for p in b.FindFootprintByReference('SW2').Pads() if p.GetNumber()=='1']
    line(b,swpads[0].GetNetname(),[xy(p) for p in swpads],.2)
    line(b,pad(b,'R4',2).GetNetname(),[xy(pad(b,'R4',2)),(37.665,130.51),(37.8,130.375)],.2)
    # Filtered USB VBUS backbone to the source mux (local routing in power module).
    line(b,'VBUS',[(61.7,68.825),(61.7,69.7),(61.3,70.1),(47.71,70.1)],.6)
    # Status LED local anode wiring and a separate short ground drop per LED.
    resistors=['R70','R69','R68','R67','R66','R65','R64','R63','R74','R73','R71']
    routes=[]
    for ref in resistors:
        rp=pad(b,ref,2);nn=rp.GetNetname();target=next(p for f in b.GetFootprints() if f.GetReference().startswith('D') for p in f.Pads() if p.GetNetname()==nn)
        a=xy(target);c=xy(rp);line(b,nn,[a,(a[0],c[1]-abs(c[0]-a[0])),c],.25)
        gp=pad(b,target.GetParentFootprint().GetReference(),1);g=xy(gp);gv=(g[0],56.4);line(b,'GND',[g,gv],.3);via(b,'GND',gv)
        ip=pad(b,ref,1);end=(xy(ip)[0],59.8);line(b,ip.GetNetname(),[xy(ip),end],.2);via(b,ip.GetNetname(),end)
        source=next(t for t in b.GetTracks() if isinstance(t,k.PCB_VIA) and t.GetNetCode()==ip.GetNetCode() and xy(t)[1]>70)
        routes.append((ip.GetNetname(),xy(source),end))
    # Use the existing GPIO via breakouts and keep both ground-reference layers intact.
    if not backbone:return routes
    router=GridRouter(b,k.In2_Cu,grid=.1,width=.2,clearance=.22)
    for nn,a,c in routes:
        pts=router.route(nn,a,c);print('LED route',nn,len(pts)-1,'segments',flush=True)
    source=next(t for t in b.GetTracks() if isinstance(t,k.PCB_VIA) and t.GetNetname()=='/Indicator Panel/BTN_START')
    router.route('/Indicator Panel/BTN_START',xy(source),xy(pad(b,'SW3',2)))
    print('Remaining-region routing applied',flush=True)

if __name__=='__main__':
    b=k.LoadBoard(str(ROOT/'opensync.kicad_pcb'));apply(b)
    target=ROOT/'tmp'/'routing_work'/'main_draft.kicad_pcb';k.SaveBoard(str(target),b)
    render(b,ROOT/'tmp'/'routing_work'/'main_preview.png')
    print(target)
