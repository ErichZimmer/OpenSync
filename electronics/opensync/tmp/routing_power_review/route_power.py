"""Native KiCad power routing. Import apply(board); this never saves the main board."""
import pcbnew as k

def apply(board):
    fps = {f.GetReference(): f for f in board.GetFootprints()}
    def point(xy): return k.VECTOR2I(k.FromMM(xy[0]), k.FromMM(xy[1]))
    def xy(p): return (k.ToMM(p.x), k.ToMM(p.y))
    def pad(ref, num): return next(p for p in fps[ref].Pads() if p.GetNumber() == str(num))
    def pos(ref, num): return xy(pad(ref,num).GetPosition())
    def code(net): return board.FindNet(net).GetNetCode()
    def trace(net, pts, width=.2, layer=k.F_Cu):
        for a,b in zip(pts,pts[1:]):
            if a==b: continue
            t=k.PCB_TRACK(board); t.SetStart(point(a)); t.SetEnd(point(b))
            t.SetWidth(k.FromMM(width)); t.SetLayer(layer); t.SetNetCode(code(net)); board.Add(t)
    def link(ref,num,pts,width=.2,layer=k.F_Cu): trace(pad(ref,num).GetNetname(),[pos(ref,num)]+pts,width,layer)
    def via(net,p,diam=.6,drill=.3):
        v=k.PCB_VIA(board); v.SetPosition(point(p)); v.SetWidth(k.FromMM(diam)); v.SetDrill(k.FromMM(drill))
        v.SetViaType(k.VIATYPE_THROUGH); v.SetLayerPair(k.F_Cu,k.B_Cu); v.SetNetCode(code(net)); board.Add(v)
    def zone(net,pts,priority=20):
        z=k.ZONE(board); z.SetLayer(k.F_Cu); z.SetNetCode(code(net)); z.SetLocalClearance(k.FromMM(.18))
        z.SetPadConnection(k.ZONE_CONNECTION_FULL); z.SetMinThickness(k.FromMM(.1)); z.SetAssignedPriority(priority)
        z.SetIslandRemovalMode(k.ISLAND_REMOVAL_MODE_ALWAYS)
        z.SetThermalReliefGap(k.FromMM(.2)); z.SetThermalReliefSpokeWidth(k.FromMM(.3))
        o=z.Outline(); o.NewOutline()
        for p in pts: o.Append(point(p))
        board.Add(z)
    def rect(net,x0,y0,x1,y1,priority=20): zone(net,[(x0,y0),(x1,y0),(x1,y1),(x0,y1)],priority)

    # Ordinary whole-footprint rotation/translation only. No pad definitions change.
    r79_ref=fps['R79'].Reference(); r79_ref_pos=k.VECTOR2I(r79_ref.GetPosition()); r79_ref_angle=r79_ref.GetTextAngle()
    fps['R79'].Rotate(fps['R79'].GetPosition(),k.EDA_ANGLE(180,k.DEGREES_T))
    r79_ref.SetPosition(r79_ref_pos); r79_ref.SetTextAngle(r79_ref_angle)
    fps['L2'].SetPosition(point((48.6,81.3)))

    # U31: dual-source mux, each input locally bypassed, dual output joined.
    vbus='VBUS'; vdc='/Power Interface/VDC'; vout='/Power Interface/VOUT'
    link('U31',3,[(46.3,71.05),pos('C68',2)],.25)
    link('C68',2,[(47.71,70.715),(47.71,70.1)],.3)
    link('U31',5,[(44.8,70.55),(44.8,71.05),pos('U31',3)],.2)
    link('U31',4,[(46.0,70.55),pos('R75',2)],.2)
    link('R75',2,[(46.69,69.11),pos('R76',1)],.2)
    link('R76',2,[(48.5,68.09)],.25); via('GND',(48.5,68.09))
    link('U31',6,[(43.4,71.05),pos('C67',2)],.3)
    link('J14',1,[(45.0,66.0),(42.5,68.5),pos('C67',2)],1.0)
    link('U31',2,[pos('U31',7)],.3)
    trace(vout,[(44.8,71.55),(44.8,72.75),pos('C79',1)],.3)
    link('C79',1,[(44.715,73.9)],.3)
    trace(vout,[(44.715,73.9),(45.75,74.935),pos('C69',2)],.8)
    link('U31',1,[(46.2,72.71),(46.2,73.6)],.3); via('GND',(46.2,73.6))
    link('C68',1,[(47.1,73.7)],.4); via('GND',(47.1,73.7))
    link('C67',1,[(42.5,73.7)],.4); via('GND',(42.5,73.7))
    link('C79',2,[(43.6,74.1)],.3); via('GND',(43.6,74.1))
    rect('GND',41.6,72.45,43.95,74.7)
    rect('GND',45.25,72.35,48.0,74.4)
    # Source power pours remain within the local mux capacitor region.
    rect(vbus,46.15,70.7,47.9,71.9)
    rect(vdc,41.6,70.7,43.45,71.9)

    # U32: high-current commutation loop stays entirely on the top layer.
    link('C69',2,[(45.75,79.75),pos('U32',10)],.35)
    link('C69',2,[(45.75,76.7),(42.7,76.7),(42.7,80.3),pos('U32',1)],.2)
    link('U32',1,[pos('U32',2)],.2)
    link('C69',1,[(44.65,78.95),(44.65,83.75),pos('C70',1),pos('C78',1)],.5)
    link('U32',8,[(44.65,81.3),pos('U32',3)],.25)
    link('U32',9,[(46.6,80.8)],.25)
    trace(pad('U32',9).GetNetname(),[(46.6,80.8),(47.15,80.25),pos('L2',1)],.6)
    link('U32',7,[(46.6,81.8)],.25)
    trace(pad('U32',7).GetNetname(),[(46.6,81.8),(47.15,82.35),pos('L2',2)],.6)
    link('U32',6,[(45.75,82.8)],.35)
    trace('+5V',[(45.75,82.8),pos('C70',2),pos('C78',2)],.8)
    link('U32',4,[(42.8,81.8),(42.29,82.31),pos('R78',1),pos('R77',1)],.2)
    # Quiet divider return joins AGND directly beside the IC, outside the capacitor power return.
    link('R78',2,[(41.31,81.3),pos('U32',3)],.25)
    link('R77',2,[(40.4,84.31)],.25); via('+5V',(40.4,84.31))
    for p in [(44.65,79.4),(44.65,82.8),(43.1,84.6),(43.1,86.8)]: via('GND',p)
    for p in [(46.8,84.6),(46.8,86.8)]:
        via('+5V',p); trace('+5V',[(45.75,p[1]),p],.7)
    rect('GND',43.0,77.1,45.05,87.8)
    rect('GND',40.0,80.55,42.2,81.7)
    zone(vout,[(45.15,77.1),(46.25,77.1),(46.25,80.45),(45.15,80.45)])
    zone('+5V',[(45.1,82.18),(46.2,82.18),(46.2,83.5),(47.3,83.5),(47.3,87.7),(45.0,87.7),(45.0,83.1)])

    # U37: input bypass -> VIN/PGND, output returns -> PGND without vias.
    link('U37',6,[(43.5,94.75),pos('C73',2)],.25)
    link('C73',2,[(44.435,94.25),pos('C71',2),pos('C72',2)],.6)
    link('U37',4,[pos('C73',1)],.25)
    link('C73',1,[(44.435,96.15),pos('C71',1),pos('C72',1)],.6)
    link('U37',10,[(41.125,94.2),pos('U37',1)],.2)
    link('U37',10,[(40.4,93.2)],.2); via('+5V',(40.4,93.2))
    link('U37',2,[(40.45,95.25)],.2)
    trace(pad('U37',2).GetNetname(),[(40.45,95.25),(39.8,94.915),pos('L3',1)],.6)
    link('L3',2,[(40.435,97.285),pos('C74',2),pos('C75',2),pos('C76',2)],.8)
    link('U37',3,[(41.125,97.5),pos('C74',2)],.2)
    link('U37',7,[(43.3,94.075)],.2); via('GND',(43.3,94.075))
    link('R80',1,[pos('R79',1)],.2)
    link('R80',1,[(41.75,92.76),pos('U37',9)],.2)
    link('C77',2,[(43.615,93.4),(42.25,93.4),pos('U37',8)],.2)
    link('R79',2,[(43.4,91.2)],.25); via('GND',(43.4,91.2))
    link('C77',1,[(44.8,91.985)],.25); via('GND',(44.8,91.985))
    # Output divider feed is already on the regulated top plane; midpoint is a short local trace.
    for p in [(44.4,97.0),(44.4,98.5),(44.4,100.5),(44.4,102.0),(48.3,96.2)]: via('GND',p)
    for p in [(48.6,94.25),(46.7,93.1)]: via('+5V',p)
    link('C72',2,[(48.6,94.25)],.7)
    trace('+5V',[(46.7,93.1),(46.7,94.25)],.6)
    rect('+5V',43.3,93.55,49.0,95.0)
    rect('GND',42.3,95.5,48.9,102.8)
    rect('GND',42.1,90.8,45.3,93.4)
    return {'power_ics':['U31','U32','U37'],'moved':['R79 (rotate 180 degrees)','L2 (x=48.6,y=81.3)'],'boundaries':{'VBUS':'C68.2 and FB1.1 require upstream join','VDC':'J14.1 now joined to C67.2/U31.6','GND':'Through vias to existing In1/B zones','+5V':'Through vias to existing In2 zone','+3V3':'U37 output joins existing F.Cu zone'}}

if __name__=='__main__':
    import sys
    board=k.LoadBoard(sys.argv[1]); print(apply(board)); k.ZONE_FILLER(board).Fill(board.Zones()); k.SaveBoard(sys.argv[2],board)
