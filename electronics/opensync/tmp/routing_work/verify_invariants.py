"""Compare routed candidate against the untouched saved-board baseline."""
import json,sys,math
from pathlib import Path
import pcbnew as k
ROOT=Path(__file__).resolve().parents[2]
def pair(v):return (v.x,v.y)
def pad_data(p):
    return {
        'number':p.GetNumber(),'net':p.GetNetname(),'relative_position':pair(p.GetFPRelativePosition()),
        'relative_angle':round(p.GetFPRelativeOrientation().AsDegrees()%360,6),
        'size':pair(p.GetSize()),'drill':pair(p.GetDrillSize()),'drill_shape':p.GetDrillShape(),
        'shape':p.GetShape(),'attribute':p.GetAttribute(),'layers':p.GetLayerSet().FmtBin(),
        'roundrect_ratio':p.GetRoundRectRadiusRatio(),'chamfer_ratio':p.GetChamferRectRatio(),
        'clearance':p.GetLocalClearance(),'thermal_gap':p.GetLocalThermalGapOverride(),
        'thermal_spoke':p.GetLocalThermalSpokeWidthOverride(),
    }
def data(b):
    return {f.GetReference():{
        'uuid':f.m_Uuid.AsString(),'position':pair(f.GetPosition()),'angle':f.GetOrientationDegrees(),
        'value':f.GetValue(),'library':f.GetFPIDAsString(),'layer':f.GetLayer(),
        'fields':dict(f.GetFieldsText()),
        'pads':{p.m_Uuid.AsString():pad_data(p) for p in f.Pads()},
        'models':[(m.m_Filename,(m.m_Scale.x,m.m_Scale.y,m.m_Scale.z),(m.m_Rotation.x,m.m_Rotation.y,m.m_Rotation.z),(m.m_Offset.x,m.m_Offset.y,m.m_Offset.z)) for m in f.Models()],
    } for f in b.GetFootprints()}
baseline=k.LoadBoard(str(ROOT/'tmp/routing_work/baseline.kicad_pcb'))
candidate=k.LoadBoard(sys.argv[1] if len(sys.argv)>1 else str(ROOT/'tmp/routing_work/integrated.kicad_pcb'))
a,c=data(baseline),data(candidate)
assert a.keys()==c.keys(),'Footprint references differ'
transforms=[]
usb_changes=[]
allowed_usb={('J13','2'):('Net-(D14-I{slash}O_1)','Net-(D14-I{slash}O_2)'),('J13','3'):('Net-(D14-I{slash}O_2)','Net-(D14-I{slash}O_1)'),('L4','1'):('/Microcontroller Circuitry/USB_D-','/Microcontroller Circuitry/USB_D+'),('L4','4'):('/Microcontroller Circuitry/USB_D+','/Microcontroller Circuitry/USB_D-')}
for ref in a:
    for prop in a[ref]:
        if prop in ('position','angle'):continue
        if prop=='pads':
            for uid,pd in a[ref]['pads'].items():
                nd=c[ref]['pads'][uid]
                if pd['net']!=nd['net']:
                    assert allowed_usb.get((ref,pd['number']))==(pd['net'],nd['net']),f'Unexpected pad-net change {ref}.{pd["number"]}'
                    usb_changes.append({'reference':ref,'pad':pd['number'],'before':pd['net'],'after':nd['net']})
                    pd['net']=nd['net']
        assert a[ref][prop]==c[ref][prop],f'{ref}: changed {prop}\n{a[ref][prop]}\n{c[ref][prop]}'
    if (a[ref]['position'],a[ref]['angle'])!=(c[ref]['position'],c[ref]['angle']):
        transforms.append({'reference':ref,'before':{x:a[ref][x] for x in ('position','angle')},'after':{x:c[ref][x] for x in ('position','angle')}})
assert {t['reference'] for t in transforms}<={'L2','R79','R65'}
def drawing(b):
    return [(d.m_Uuid.AsString(),str(d.GetShape()),pair(d.GetStart()),pair(d.GetEnd()),d.GetWidth()) for d in b.GetDrawings() if d.GetLayer()==k.Edge_Cuts]
assert drawing(baseline)==drawing(candidate),'Board outline changed'
original_copper={t.m_Uuid.AsString():t for t in baseline.GetTracks()}
candidate_copper={t.m_Uuid.AsString():t for t in candidate.GetTracks()}
for uid in original_copper.keys() & candidate_copper.keys():
    old,new=original_copper[uid],candidate_copper[uid]
    if old.GetNetname()!=new.GetNetname():
        assert {old.GetNetname(),new.GetNetname()}=={'Net-(D14-I{slash}O_1)','Net-(D14-I{slash}O_2)'},f'Original non-USB copper net changed: {uid}'
    assert old.GetLayerSet().FmtBin()==new.GetLayerSet().FmtBin(),f'Original copper layers changed: {uid}'
    if isinstance(old,k.PCB_VIA):
        assert old.GetWidth(k.F_Cu)==new.GetWidth(k.F_Cu) and old.GetDrill()==new.GetDrill(),f'Original via geometry changed: {uid}'
    else:assert old.GetWidth()==new.GetWidth(),f'Original track width changed: {uid}'
removed=set(original_copper)-set(candidate_copper)
from cleanup_routes import STUBS
for uid in removed-STUBS:
    old=original_copper[uid];nn=old.GetNetname()
    assert nn in ['Net-(D14-I{slash}O_1)','Net-(D14-I{slash}O_2)','/Microcontroller Circuitry/USB_D+','/Microcontroller Circuitry/USB_D-'] or (isinstance(old,k.PCB_VIA) and nn.startswith('/Indicator Panel/LED_')),f'Unexpected removed original copper {uid} {nn}'
report={'footprints':len(a),'pad_definitions_unchanged':True,'fields_3d_models_unchanged':True,'outline_unchanged':True,'permitted_rigid_transforms':transforms,'before_tracks_and_vias':len(list(baseline.GetTracks())),'after_tracks_and_vias':len(list(candidate.GetTracks())),'before_zones':len(list(baseline.Zones())),'after_zones':len(list(candidate.Zones()))}
report['original_non_usb_copper_nets_widths_layers_unchanged']=True
report['removed_original_copper_count']=len(removed)
report['authorized_usb_pad_net_changes']=usb_changes
print(json.dumps(report,indent=2,default=str))
(ROOT/'tmp/routing_work/invariants.json').write_text(json.dumps(report,indent=2,default=str))
