import pcbnew as p
import json
from collections import defaultdict

b=p.LoadBoard(r'C:\Users\Research\repos\OpenSync\electronics\opensync\opensync\opensync.kicad_pcb')
mm=lambda q: [round(p.ToMM(q.x),6),round(p.ToMM(q.y),6)]
fps={f.GetReference():f for f in b.GetFootprints()}
rows=[]
for f in b.GetFootprints():
    sheet=f.GetSheetname()
    if 'Output Channel' in sheet:
        rows.append(dict(ref=f.GetReference(),value=f.GetValue(),lib=f.GetFPID().GetUniStringLibId(),sheet=sheet,path=f.GetPath().AsString(),pos=mm(f.GetPosition()),angle=f.GetOrientationDegrees(),pads=[dict(n=x.GetNumber(),pos=mm(x.GetPosition()),net=x.GetNetname(),size=mm(x.GetSize()),angle=x.GetOrientationDegrees()) for x in f.Pads()]))
rows.sort(key=lambda r:(r['sheet'],r['ref']))
tracks=[dict(type=t.GetClass(),uuid=t.m_Uuid.AsString(),net=t.GetNetname(),start=mm(t.GetStart()),end=mm(t.GetEnd()),width=p.ToMM(t.GetWidth()),layer=b.GetLayerName(t.GetLayer())) for t in b.GetTracks()]
zones=[dict(net=z.GetNetname(),layer=b.GetLayerName(z.GetLayer()),n=z.Outline().TotalVertices()) for z in b.Zones()]
print(json.dumps(dict(footprints=rows,tracks=tracks,zones=zones),indent=2))
