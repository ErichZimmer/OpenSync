"""MCU-local and USB routing. Uses only board items; does not mutate pad geometry.

apply(board) is the integration entry point. Running this module saves/fills a
scratch board only; the project PCB is never saved by this script.
"""
import pcbnew as p
import os
import shutil
import sys

def mm(v): return p.FromMM(v)
def point(x,y): return p.VECTOR2I(mm(x),mm(y))

def add_vin_feed(board):
    """Reserve the MCU VIN feed before the later LED-backbone router runs."""
    net=next(n for n in board.GetNetsByNetcode().values() if n.GetNetname()=='+3V3')
    for x,y in [(87.6,84.2),(91.7,82.8)]:
        vi=p.PCB_VIA(board);vi.SetPosition(point(x,y));vi.SetWidth(mm(.6));vi.SetDrill(mm(.3));vi.SetViaType(p.VIATYPE_THROUGH);vi.SetLayerPair(p.F_Cu,p.B_Cu);vi.SetNet(net);board.Add(vi)
    paths=[(p.F_Cu,.15,[(87.6,83.1035),(87.6,84.2)]),
           (p.In2_Cu,.3,[(87.6,84.2),(90.3,84.2),(91.7,82.8)]),
           (p.F_Cu,.25,[(91.7,82.8),(91.7,83.415),(91.985,83.7)])]
    for layer,width,pts in paths:
        for a,b in zip(pts,pts[1:]):
            tr=p.PCB_TRACK(board);tr.SetStart(point(*a));tr.SetEnd(point(*b));tr.SetWidth(mm(width));tr.SetLayer(layer);tr.SetNet(net);board.Add(tr)
    return {'tracks':5,'vias':2}

def remap_usb(board):
    """Apply the USB-only, schematic-authorized equivalent-channel remap."""
    nets={n.GetNetname():n for n in board.GetNetsByNetcode().values()}
    assignments={('J13','2'):'Net-(D14-I{slash}O_2)',
                 ('J13','3'):'Net-(D14-I{slash}O_1)',
                 ('L4','1'):'/Microcontroller Circuitry/USB_D+',
                 ('L4','4'):'/Microcontroller Circuitry/USB_D-'}
    count=0
    for f in board.GetFootprints():
        for pad in f.Pads():
            key=(f.GetReference(),pad.GetNumber())
            if key in assignments:
                pad.SetNet(nets[assignments[key]]);count+=1
    reassigned=0
    for t in board.GetTracks():
        if t.m_Uuid.AsString()=='3f0e1258-28a3-49f9-afb3-423757a40fbe':
            t.SetNet(nets['Net-(D14-I{slash}O_2)']);reassigned+=1
    return {'pads':count,'existing_tracks':reassigned}

def apply(board):
    fps={f.GetReference():f for f in board.GetFootprints()}
    nets={n.GetNetname():n for n in board.GetNetsByNetcode().values()}
    counts={'tracks':0,'vias':0,'zones':0,'removed_tails':0}
    counts['usb_remap']=remap_usb(board)
    def track(net,pts,width=.15,layer=p.F_Cu):
        for a,b in zip(pts,pts[1:]):
            if a==b:continue
            tr=p.PCB_TRACK(board)
            tr.SetStart(point(*a));tr.SetEnd(point(*b));tr.SetWidth(mm(width));tr.SetLayer(layer);tr.SetNet(nets[net]);board.Add(tr)
            counts['tracks']+=1
    def via(net,x,y):
        vi=p.PCB_VIA(board);vi.SetPosition(point(x,y));vi.SetWidth(mm(.6));vi.SetDrill(mm(.3));vi.SetViaType(p.VIATYPE_THROUGH);vi.SetLayerPair(p.F_Cu,p.B_Cu);vi.SetNet(nets[net]);board.Add(vi)
        counts['vias']+=1
    def zone(net,pts,layer,priority,name,keepout=False):
        z=p.ZONE(board);z.SetLayer(layer);z.SetZoneName(name)
        z.SetLocalClearance(mm(.18));z.SetMinThickness(mm(.1));z.SetAssignedPriority(priority)
        if keepout:
            z.SetIsRuleArea(True);z.SetDoNotAllowZoneFills(True);z.SetDoNotAllowTracks(True);z.SetDoNotAllowVias(True);z.SetDoNotAllowPads(False);z.SetDoNotAllowFootprints(False)
        else:
            z.SetNet(nets[net]);z.SetPadConnection(p.ZONE_CONNECTION_FULL)
        poly=z.Outline();poly.NewOutline()
        for x,y in pts:poly.Append(mm(x),mm(y))
        board.Add(z);counts['zones']+=1
        return z

    # Complete the boot-select via-to-resistor link without changing its route.
    track('/Microcontroller Circuitry/QSPI_SS',[(36.5,128.7),(36.5,129.49)],.2)

    # Existing LX copper is a translated Raspberry Pi reference shape but had
    # lost its net assignment. Restore its electrical purpose with the API.
    for z in board.Zones():
        bb=z.GetBoundingBox()
        if not z.GetNetname() and 87<p.ToMM(bb.GetX())<89 and 77<p.ToMM(bb.GetY())<80 and not z.GetIsRuleArea():
            z.SetNet(nets['/Microcontroller Circuitry/VREG_LX']);z.SetZoneName('RP2354 VREG LX local copper');z.SetPadConnection(p.ZONE_CONNECTION_FULL)
    # Keep the small switching node free of copper on the immediately adjacent
    # layer, as required by RP2350 DS section 6.3.8.
    zone('',[(86.6,76.9),(89.4,76.9),(89.4,80.0),(88.3,80.45),(88.3,83.4),(87.7,83.4),(87.7,79.6),(86.6,79.6)],p.In1_Cu,0,'RP2354 LX L2 copper keepout',True)
    # High-current ground is already a local zone shared by CIN, COUT and PGND.
    # Two neighboring vias tie it to the reference plane at a single location.
    via('GND',89.05,80.85);via('GND',89.05,81.45)
    track('GND',[(88.515,80.7),(88.9,80.7),(89.05,80.85)],.25)
    track('GND',[(88.515,81.7),(88.8,81.7),(89.05,81.45)],.25)
    # Filtered AVDD receives a separate ground return, away from the hot loop.
    via('GND',90.5,82.55)
    track('GND',[(90.2,81.915),(90.2,82.25),(90.5,82.55)],.25)
    # The pre-existing crystal guard copper had no plane connection. Give each
    # load capacitor a direct local ground via; retain all oscillator traces.
    via('GND',82.5,98.4);via('GND',87.7,98.4)
    track('GND',[(83.085,98.4),(82.5,98.4)],.25)
    track('GND',[(87.115,98.4),(87.7,98.4)],.25)
    track('Net-(U1-VREG_AVDD)',[(88.8,83.1035),(88.8,83.0),(89.65,82.15),(89.65,81.35),(90.115,80.885),(90.2,80.885)],.2)
    track('Net-(U1-VREG_AVDD)',[(90.2,80.885),(90.2,79.91),(90.8,79.31),(91.4,79.31)],.2)
    # Keep feedback on the capacitor/output side, clear of the LX node.
    track('+1V1',[(87.2,83.1035),(87.2,82.35),(86.6,81.75),(86.6,81.1)],.1)
    track('+3V3',[(87.6,83.1035),(87.6,82.15),(87.485,82.035),(87.485,81.7)],.15)
    # USB/OTP and QSPI supply pins to their local bypass C9; join same-supply
    # neighboring pins without changing any package pads.
    track('+3V3',[(86.0,83.1035),(86.0,82.8),(85.6,82.8)],.15)
    track('+3V3',[(85.6,83.1035),(85.6,82.6),(84.0,81.0),(84.0,79.715)],.15)
    track('+3V3',[(84.0,79.715),(84.0,80.6),(83.8,80.8),(83.8,81.8),(83.685,81.915),(82.6,81.915)],.2)
    track('+3V3',[(89.8965,84.6),(89.8965,84.2)],.15)

    # USB series damping resistors stay close to the MCU. Both traces remain
    # on F.Cu over In1.Cu ground through the fine-pitch escape.
    track('Net-(U1-USB_DP)',[(86.4,83.1035),(86.4,82.5),(85.0,81.1),(85.0,79.71)],.15)
    track('Net-(U1-USB_DM)',[(86.8,83.1035),(86.8,82.4),(86.0,81.6),(86.0,79.71)],.15)
    dp='/Microcontroller Circuitry/USB_D+'
    dm='/Microcontroller Circuitry/USB_D-'
    # Long pair: .15 mm width / .20 mm nominal gap on the 3313 outer stackup.
    # Equivalent choke/ESD channels are swapped in the authorized USB schematic,
    # allowing this complete pair to stay on F.Cu with no signal transition vias.
    track(dp,[(64.5,77.35),(64.5,78.4),(65.8,78.4),(66.8,77.4),(84.55,77.4),(85.0,77.85),(85.0,78.69)],.15)
    track(dm,[(65.5,77.35),(65.8,77.05),(84.694975,77.05),(86.0,78.355025),(86.0,78.69)],.15)

    # The original north tail now leads to the correct D+ connector pin after
    # the channel swap; retain it and finish its last fraction of a millimeter.
    raw_dp='Net-(D14-I{slash}O_1)';raw_dm='Net-(D14-I{slash}O_2)'
    track(raw_dp,[(64.467136,62.332864),(65.09,61.71),(65.25,61.71)],.2)
    track(raw_dm,[(65.25,64.35),(65.433401,64.533401),(65.433401,64.704)],.2)

    # Raw USB power: branch to TVS before the filter, with a short inner-layer
    # crossing under the USB pair. FB1 filtered side belongs to the power block.
    raw='/USB Interface/VBUS_RAW'
    track(raw,[(62.75,63.71),(62.75,64.8),(61.7,65.85),(61.7,67.175)],.5)
    track(raw,[(61.7,67.175),(62.325,67.175),(62.7,66.8)],.5)
    via(raw,62.7,66.8);via(raw,67.0,68.1)
    track(raw,[(62.7,66.8),(64.0,68.1),(67.0,68.1)],.5,p.In2_Cu)
    track(raw,[(67.0,68.1),(67.0,69.3),(66.25,70.05)],.5)
    vin=add_vin_feed(board)
    counts['tracks']+=vin['tracks'];counts['vias']+=vin['vias']
    board.BuildConnectivity()
    return counts

if __name__=='__main__':
    src=sys.argv[1] if len(sys.argv)>1 else 'opensync.kicad_pcb'
    out=sys.argv[2] if len(sys.argv)>2 else 'tmp/routing_digital_review/digital_candidate.kicad_pcb'
    board=p.LoadBoard(src)
    print(apply(board))
    p.ZONE_FILLER(board).Fill(board.Zones())
    p.SaveBoard(out,board)
    pro=os.path.splitext(src)[0]+'.kicad_pro'
    if os.path.isfile(pro):shutil.copyfile(pro,os.path.splitext(out)[0]+'.kicad_pro')
