import json
import sys
from pathlib import Path
from collections import Counter
import pcbnew as k

ROOT = Path(__file__).resolve().parents[2]
path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'opensync.kicad_pcb'
b = k.LoadBoard(str(path))
def mm(p):
    return [round(k.ToMM(p.x), 6), round(k.ToMM(p.y), 6)]
def uid(x):
    return x.m_Uuid.AsString()
fps=[]
for f in b.GetFootprints():
    pads=[]
    for p in f.Pads():
        pads.append(dict(number=p.GetNumber(),uuid=uid(p),xy=mm(p.GetPosition()),size=mm(p.GetSize()),angle=p.GetOrientationDegrees(),shape=int(p.GetShape()),layers=[b.GetLayerName(i) for i in p.GetLayerSet().Seq()],net=p.GetNetname(),netcode=p.GetNetCode(),drill=mm(p.GetDrillSize()),attribute=int(p.GetAttribute())))
    fps.append(dict(ref=f.GetReference(),value=f.GetValue(),uuid=uid(f),xy=mm(f.GetPosition()),angle=f.GetOrientationDegrees(),layer=b.GetLayerName(f.GetLayer()),footprint=f.GetFPID().GetLibItemName(),sheet=f.GetSheetname(),sheetfile=f.GetSheetfile(),path=f.GetPath().AsString(),pads=pads))
tracks=[]
for t in b.GetTracks():
    d=dict(uuid=uid(t),net=t.GetNetname(),netcode=t.GetNetCode(),width=k.ToMM(t.GetWidth(k.F_Cu) if isinstance(t,k.PCB_VIA) else t.GetWidth()),layer=b.GetLayerName(t.GetLayer()),start=mm(t.GetStart()),end=mm(t.GetEnd()),type=t.GetClass(),locked=t.IsLocked())
    if isinstance(t,k.PCB_VIA):
        d.update(drill=k.ToMM(t.GetDrillValue()),layers=[b.GetLayerName(i) for i in t.GetLayerSet().Seq()])
    tracks.append(d)
zones=[]
for z in b.Zones():
    pol=z.Outline()
    outlines=[]
    for i in range(pol.OutlineCount()):
        o=pol.COutline(i)
        outlines.append([mm(o.CPoint(j)) for j in range(o.PointCount())])
    zones.append(dict(uuid=uid(z),net=z.GetNetname(),netcode=z.GetNetCode(),layers=[b.GetLayerName(i) for i in z.GetLayerSet().Seq()],rule=z.GetIsRuleArea(),priority=z.GetAssignedPriority(),clearance=k.ToMM(z.GetLocalClearance()),outlines=outlines))
bb=b.GetBoardEdgesBoundingBox()
summary=dict(version=k.GetBuildVersion(),file=str(path),bounds=[mm(bb.GetOrigin()),mm(bb.GetEnd())],footprints=len(fps),tracks=Counter(t['type'] for t in tracks),track_layers=Counter(t['layer'] for t in tracks),zones=len(zones),nets=len(b.GetNetsByNetcode()))
data=dict(summary=summary,footprints=fps,tracks=tracks,zones=zones,nets={str(n):v.GetNetname() for n,v in b.GetNetsByNetcode().items()})
out=ROOT/'tmp'/'routing_work'/('board_dump.json' if len(sys.argv)<3 else sys.argv[2])
out.write_text(json.dumps(data,indent=2,default=str),encoding='utf-8')
print(json.dumps(summary,indent=2))
print('zones:',json.dumps(zones,indent=2,default=str))
print('dump:',out)
