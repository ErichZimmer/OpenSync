"""Validate the LED-only diff, then optionally save with native KiCad API."""
import sys,json,hashlib,math
from pathlib import Path
from collections import Counter
import pcbnew as k
from organize_leds import ROOT,WORK,NAMES,SOURCES,DESTS,net,uid,xy
sys.path.insert(0,'C:/Users/Research/.codex/skills/kicad/scripts')
from sexp_parser import parse_file

def copper(board):
    out={}
    for t in board.GetTracks():
        v=isinstance(t,k.PCB_VIA)
        out[uid(t)]={'net':t.GetNetname(),'type':type(t).__name__,
            'start':xy(t.GetStart()),'end':xy(t.GetEnd()),
            'width':t.GetWidth(k.F_Cu) if v else t.GetWidth(),
            'drill':t.GetDrillValue() if v else None,
            'layers':t.GetLayerSet().FmtBin(),'locked':t.IsLocked()}
    return out

def fixed_blocks(path):
    doc=parse_file(str(path))
    return sorted(json.dumps(v,sort_keys=True) for v in doc if isinstance(v,list)
        and v[0] not in ['segment','arc','via','zone','generator','generator_version'])

def copper_blocks(path,include):
    out={}
    for block in parse_file(str(path)):
        if not isinstance(block,list) or block[0] not in ['segment','arc','via']:continue
        u=next(a[1] for a in block[1:] if isinstance(a,list) and a[0]=='uuid')
        if u in include:out[u]=json.dumps(block,sort_keys=True)
    return out

def zone_definitions(path):
    doc=parse_file(str(path))
    skip={'filled_polygon','fill_segments','fill_version','filled_areas_thickness'}
    return sorted(json.dumps([v[0]]+[a for a in v[1:]
        if not(isinstance(a,list) and a[0] in skip)],sort_keys=True)
        for v in doc if isinstance(v,list) and v[0]=='zone')

def stats(b):
    totals=Counter();nets={}
    for name in sorted(NAMES):
        ts=[t for t in b.GetTracks() if t.GetNetname()==name]
        vs=[t for t in ts if isinstance(t,k.PCB_VIA)]
        segs=[t for t in ts if not isinstance(t,k.PCB_VIA)]
        n={'segments':len(segs),'vias':len(vs),'length_mm':round(sum(k.ToMM(t.GetLength()) for t in segs),3)}
        nets[name]=n;totals.update(n)
    return {'totals':dict(totals),'nets':nets}

def check(candidate):
    source=WORK/'source.kicad_pcb'
    b0=k.LoadBoard(str(source));b1=k.LoadBoard(str(candidate))
    c0,c1=copper(b0),copper(b1)
    fixed0={u:t for u,t in c0.items() if t['net'] not in NAMES}
    fixed1={u:t for u,t in c1.items() if t['net'] not in NAMES}
    assert fixed0==fixed1,'Non-LED copper changed'
    assert copper_blocks(source,set(fixed0))==copper_blocks(candidate,set(fixed1)),'Non-LED copper properties changed'
    assert fixed_blocks(source)==fixed_blocks(candidate),'Footprint/pad/model or other fixed board data changed'
    assert zone_definitions(source)==zone_definitions(candidate),'Zone settings or outlines changed'
    for name,pt in SOURCES.items():
        for target in [pt,(DESTS[name],59.8)]:
            old=[u for u,t in c0.items() if t['type']=='PCB_VIA' and t['net']==net(name) and t['start']==target]
            assert len(old)==1,('Expected original terminal via',name,target)
            if old[0] in c1:
                assert c1[old[0]]==c0[old[0]],('Retained terminal via changed',name,target)
            else:
                # A source via may be removed only when routing now stays on F.Cu.
                assert target==pt,('LED-row terminal via removed',name,target)
                former=next(t for t in b0.GetTracks() if uid(t)==old[0])
                shape=former.GetEffectiveShape(k.In2_Cu)
                assert not any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==net(name)
                    and t.GetLayer()==k.In2_Cu and shape.Collide(t.GetEffectiveShape(k.In2_Cu),0)
                    for t in b1.GetTracks()),('Source via still required',name,target)
    s0,s1=stats(b0),stats(b1)
    assert s1['totals']['vias'] < s0['totals']['vias'],'No LED via reduction'
    assert s1['totals']['segments'] < s0['totals']['segments'],'No LED segment reduction'
    assert s1['totals']['length_mm'] < s0['totals']['length_mm'],'No LED length reduction'
    added=set(c1)-set(c0);removed=set(c0)-set(c1)
    modified={u for u in set(c0)&set(c1) if c0[u]!=c1[u]}
    report={'candidate':str(candidate),'before':s0,'after':s1,
        'added_copper':len(added),'removed_copper':len(removed),'modified_copper':len(modified),
        'footprints_pads_models_preserved':True,'non_LED_copper_preserved':True,'zone_definitions_preserved':True}
    (WORK/'validation.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report,indent=2),flush=True)
    return b1

if __name__=='__main__':
    candidate=Path(sys.argv[1]).resolve()
    b=check(candidate)
    if '--commit' in sys.argv:
        drc=json.loads(candidate.with_suffix('.drc.json').read_text())
        assert not [v for v in drc['violations'] if v['severity']=='error'],'DRC errors remain'
        assert not drc['unconnected_items'],'Unconnected copper remains'
        assert not drc.get('schematic_parity',[]),'Schematic parity issues remain'
        main=ROOT/'opensync.kicad_pcb';source=WORK/'source.kicad_pcb'
        assert hashlib.sha256(main.read_bytes()).digest()==hashlib.sha256(source.read_bytes()).digest(),'Main board changed during work; refuse overwrite'
        assert k.SaveBoard(str(main),b)
        check(main)
        print('SAVED MAIN PCB',flush=True)
