"""Add nearby return vias for LED F.Cu/In2.Cu transitions, without editing planes."""
import pcbnew as p
import math
import numpy as np
from route_channels import Collision,vec,xy

def apply(board):
    # Inspect actual filled copper, so added antipads cannot sever narrow supply
    # connections. This changes no zone boundary or pad connection setting.
    p.ZONE_FILLER(board).Fill(board.Zones())
    layers=[p.F_Cu,p.In1_Cu,p.In2_Cu,p.B_Cu]
    checks=[Collision(board,layer,'GND',width=.6,clearance=.20) for layer in layers]
    forbid=[z for z in board.Zones() if z.GetIsRuleArea() and z.GetDoNotAllowVias() and any(z.IsOnLayer(l) for l in layers)]
    localpower=[z for z in board.Zones() if not z.GetIsRuleArea() and z.GetNetname() not in ['GND',''] and z.GetAssignedPriority()>0 and p.ToMM(z.GetBoundingBox().GetWidth())<20 and p.ToMM(z.GetBoundingBox().GetHeight())<20]
    powers=[]
    for z in board.Zones():
        if z.GetIsRuleArea() or z.GetNetname() in ['GND','']:continue
        for layer in layers:
            if z.IsOnLayer(layer):
                polys=p.SHAPE_POLY_SET(z.GetFilledPolysList(layer))
                if not polys.IsEmpty():
                    polys.Unfracture()
                    segments=[]
                    for i in range(polys.OutlineCount()):
                        rings=[polys.COutline(i)]+[polys.CHole(i,j) for j in range(polys.HoleCount(i))]
                        for ring in rings:
                            points=[xy(ring.CPoint(j)) for j in range(ring.PointCount())]
                            segments.extend(zip(points,points[1:]+points[:1]))
                    if segments:
                        seg=np.array(segments);a=seg[:,0,:];d=seg[:,1,:]-a
                        powers.append((z,a,d,np.sum(d*d,axis=1)))
    def plane_safe(q):
        # Full antipad disk must stay strictly on one side of every filled
        # supply boundary, including holes. This cannot sever a copper neck.
        for z,a,d,dd in powers:
            radius=.3+max(.18,p.ToMM(z.GetLocalClearance()))+.03
            t=np.clip(np.divide(np.sum((np.array(q)-a)*d,axis=1),dd,out=np.zeros_like(dd),where=dd>0),0,1)
            if np.min(np.linalg.norm(a+t[:,None]*d-np.array(q),axis=1))<=radius:return False
        if any(math.dist(q,g)<1.66 for g in newgrounds):return False
        return True
    grounds=[xy(t.GetPosition()) for t in board.GetTracks() if isinstance(t,p.PCB_VIA) and t.GetNetname()=='GND']
    transitions=[t for t in board.GetTracks() if isinstance(t,p.PCB_VIA) and ('/Indicator Panel/LED_' in t.GetNetname() or t.GetNetname()=='/Indicator Panel/BTN_START')]
    transitions.sort(key=lambda t:(xy(t.GetPosition())[1],xy(t.GetPosition())[0]))
    positions=[];unresolved=[];newgrounds=[]
    for sig in transitions:
        at=xy(sig.GetPosition())
        # Do not perforate the dense MCU power islands. Existing return vias
        # there remain as designed; add stitches in open crossover/LED areas.
        if not (at[0]>95 or (76<=at[0]<=95 and 60<=at[1]<=75)):continue
        if grounds and min(math.dist(at,q) for q in grounds)<=2.0:continue
        found=None
        # Prefer cardinal offsets and rounded 0.1 mm positions.
        candidates=[]
        for radius in [.9,1.1,1.3,1.5,1.7,1.9,2.0]:
            for degree in [0,90,180,270,45,135,225,315]:
                a=math.radians(degree);q=(round(at[0]+radius*math.cos(a),1),round(at[1]+radius*math.sin(a),1))
                if q not in candidates:candidates.append(q)
        for q in candidates:
            if not((95<q[0]<=186.7 and 51.8<=q[1]<=148.2) or (76<=q[0]<=95 and 60<=q[1]<=75)):continue
            if any(math.dist(q,g)<.61 for g in grounds):continue
            shape=p.SHAPE_SEGMENT(vec(q),vec(q),p.FromMM(.6))
            if any(z.Outline().Collide(shape,p.FromMM(.05)) for z in forbid):continue
            if any(z.Outline().Collide(shape,p.FromMM(.18)) for z in localpower):continue
            if not all(c.clear(q,q) for c in checks):continue
            if not plane_safe(q):continue
            found=q
            break
        if found is None:
            unresolved.append({'net':sig.GetNetname(),'at':at});continue
        v=p.PCB_VIA(board);v.SetPosition(vec(found));v.SetViaType(p.VIATYPE_THROUGH);v.SetLayerPair(p.F_Cu,p.B_Cu)
        v.SetWidth(p.F_Cu,p.FromMM(.6));v.SetDrill(p.FromMM(.3));v.SetNetCode(board.FindNet('GND').GetNetCode());board.Add(v)
        grounds.append(found);newgrounds.append(found);positions.append({'at':found,'near':sig.GetNetname(),'signal_at':at})
    unresolved=[{'net':t.GetNetname(),'at':xy(t.GetPosition()),'nearest_gnd':round(min(math.dist(xy(t.GetPosition()),g) for g in grounds),3)} for t in transitions if min(math.dist(xy(t.GetPosition()),g) for g in grounds)>2.0]
    return {'added':len(positions),'positions':positions,'unresolved':unresolved}

if __name__=='__main__':
    from pathlib import Path
    import shutil,json,sys
    folder=Path(__file__).resolve().parent
    source=Path(sys.argv[1]) if len(sys.argv)>1 else folder/'led-draft.kicad_pcb'
    out=Path(sys.argv[2]) if len(sys.argv)>2 else folder/'led-stitched.kicad_pcb'
    b=p.LoadBoard(str(source));print(json.dumps(apply(b),indent=2))
    p.ZONE_FILLER(b).Fill(b.Zones());p.SaveBoard(str(out),b)
    shutil.copy2(source.with_suffix('.kicad_pro'),out.with_suffix('.kicad_pro'))
