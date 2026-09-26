"""Post-LED local MCU input-supply corrections. No footprint/pad mutations."""
import pcbnew as p

def apply(board):
    fixed=False
    for z in board.Zones():
        bb=z.GetBoundingBox()
        if z.GetNetname()!='+3V3' or z.GetLayer()!=p.F_Cu:
            continue
        if p.ToMM(bb.GetWidth())>100:
            # This one small pad belongs to the solid regulator VIN zone, not
            # a wide-gap thermal connection on the board-wide supply plane.
            poly=z.Outline();hole=poly.NewHole(0)
            for x,y in [(86.65,80.8),(88.32,80.8),(88.32,82.6),(86.65,82.6)]:
                poly.Append(p.FromMM(x),p.FromMM(y),0,hole)
        elif (87.0<p.ToMM(bb.GetX())<87.4
              and p.ToMM(bb.GetWidth())<1.0
              and 81.0<p.ToMM(bb.GetY())<81.5):
            poly=z.Outline();poly.RemoveAllContours();poly.NewOutline()
            for x,y in [(87.5,83.55),(87.5,82.15),(87.15,82.15),
                        (87.15,81.3),(87.75,81.3),(87.75,82.15),
                        (87.7,82.15),(87.7,83.55)]:
                poly.Append(p.FromMM(x),p.FromMM(y))
            z.SetPadConnection(p.ZONE_CONNECTION_FULL)
            z.SetZoneName('RP2354 CIN local VIN')
            fixed=True
    return {'c1_local_vin':fixed}
