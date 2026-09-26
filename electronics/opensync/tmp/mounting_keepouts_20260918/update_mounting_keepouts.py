"""Add four M3 mounting-hole copper-pour keepouts using native KiCad APIs."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

import pcbnew as k

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(__file__).resolve().parent
MAIN = ROOT / 'opensync.kicad_pcb'
BEFORE = WORK / 'before.kicad_pcb'
CANDIDATE = WORK / 'opensync.kicad_pcb'
AUDIT = WORK / 'audit.json'
TARGETS = {'H1', 'H2', 'H3', 'H4'}
FPID = 'MountingHole:MountingHole_3.2mm_M3_ISO7380'
sys.path.insert(0, r'C:\Users\Research\.codex\skills\kicad\scripts')
from sexp_parser import parse_file


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def closed():
    for name in ['pcbnew.exe', 'eeschema.exe']:
        result = subprocess.run(['tasklist', '/FI', f'IMAGENAME eq {name}', '/FO', 'CSV', '/NH'],
                                check=True, capture_output=True, text=True)
        assert name not in result.stdout.lower(), f'{name} is open; stop to protect live edits'
    assert not list(ROOT.glob('*.lck')), 'KiCad lock file exists'


def child(node, key):
    return next((x for x in node if isinstance(x, list) and x and x[0] == key), None)


def normalize(tree, added_ids):
    result = []
    for node in copy.deepcopy(tree):
        if isinstance(node, list) and node and node[0] == 'zone':
            uid = child(node, 'uuid')[1]
            if uid in added_ids:
                continue
            node[:] = [x for x in node if not (isinstance(x, list) and x and x[0] == 'filled_polygon')]
        result.append(node)
    return result


def schematic_check():
    tree = parse_file(str(ROOT / 'opensync_holes_mounting.kicad_sch'))
    refs = set()
    for symbol in tree:
        if not isinstance(symbol, list) or not symbol or symbol[0] != 'symbol':
            continue
        props = {p[1]: p[2] for p in symbol if isinstance(p, list) and p and p[0] == 'property'}
        refs.add(props['Reference'])
        assert props['Footprint'] == FPID
        assert child(symbol, 'lib_id')[1] == 'Mechanical:MountingHole'
    assert refs == TARGETS


def inspect_holes(board):
    holes = {fp.GetReference(): fp for fp in board.GetFootprints() if fp.GetReference() in TARGETS}
    assert set(holes) == TARGETS
    info = {}
    for ref, fp in holes.items():
        assert str(fp.GetFPID().GetLibNickname()) + ':' + str(fp.GetFPID().GetLibItemName()) == FPID
        pads = list(fp.Pads())
        assert len(pads) == 1
        pad = pads[0]
        assert pad.GetAttribute() == k.PAD_ATTRIB_NPTH
        assert pad.GetDrillSize() == k.VECTOR2I(k.FromMM(3.2), k.FromMM(3.2))
        assert pad.GetNetCode() == 0
        assert pad.GetPosition() == fp.GetPosition()
        pos = fp.GetPosition()
        info[ref] = {'position_mm': [k.ToMM(pos.x), k.ToMM(pos.y)],
                     'rotation_deg': fp.GetOrientationDegrees(), 'drill_mm': 3.2,
                     'footprint': FPID, 'uuid': fp.m_Uuid.AsString()}
    return holes, info


def verify(before, after, added_ids):
    assert normalize(parse_file(str(before)), set()) == normalize(parse_file(str(after)), set(added_ids.values())), \
        'Unexpected changes beyond new rule areas and recalculated zone-fill polygons'
    board = k.LoadBoard(str(after))
    _, holes = inspect_holes(board)
    assert holes == inspect_holes(k.LoadBoard(str(before)))[1]
    zones = {z.m_Uuid.AsString(): z for z in board.Zones()}
    expected_layers = {k.F_Cu, k.In1_Cu, k.In2_Cu, k.B_Cu}
    checked = []
    for ref, uid in added_ids.items():
        zone = zones[uid]
        assert zone.GetIsRuleArea() and zone.GetDoNotAllowZoneFills()
        assert not any([zone.GetDoNotAllowTracks(), zone.GetDoNotAllowVias(),
                        zone.GetDoNotAllowPads(), zone.GetDoNotAllowFootprints()])
        assert set(zone.GetLayerSet().Seq()) == expected_layers
        box = zone.GetBoundingBox()
        x, y = holes[ref]['position_mm']
        assert box.GetOrigin() == k.VECTOR2I(k.FromMM(x-5), k.FromMM(y-5))
        assert box.GetSize() == k.VECTOR2I(k.FromMM(10), k.FromMM(10))
        for fill_zone in zones.values():
            if fill_zone.GetIsRuleArea():
                continue
            for layer in fill_zone.GetLayerSet().Seq():
                if layer in expected_layers and fill_zone.HasFilledPolysForLayer(layer):
                    overlap = k.SHAPE_POLY_SET(fill_zone.GetFilledPolysList(layer))
                    overlap.BooleanIntersection(zone.Outline())
                    assert overlap.IsEmpty(), f'Copper fill intersects {ref} on layer {layer}'
                    checked.append([ref, fill_zone.m_Uuid.AsString(), board.GetLayerName(layer)])
    return {'holes': holes, 'fill_intersection_checks': len(checked), 'all_clear': True}


def prepare():
    closed()
    assert not AUDIT.exists()
    schematic_check()
    untouched = {str(p.relative_to(ROOT)): digest(p) for p in ROOT.iterdir() if p.is_file() and p != MAIN}
    original_hash = digest(MAIN)
    if BEFORE.exists():
        assert digest(BEFORE) == original_hash, 'Preserve existing backup: main board changed'
    else:
        shutil.copy2(MAIN, BEFORE)
    shutil.copy2(ROOT / 'opensync.kicad_pro', WORK / 'opensync.kicad_pro')
    board = k.LoadBoard(str(MAIN))
    holes, info = inspect_holes(board)
    assert board.GetCopperLayerCount() == 4
    layers = k.LSET.AllCuMask(4)
    assert set(layers.Seq()) == {k.F_Cu, k.In1_Cu, k.In2_Cu, k.B_Cu}
    added = {}
    for ref in sorted(TARGETS):
        pos = holes[ref].GetPosition()
        zone = k.ZONE(board)
        zone.SetIsRuleArea(True)
        zone.SetZoneName(f'{ref} M3 bracket - 10mm square copper-pour keepout')
        zone.SetLayerSet(layers)
        zone.SetDoNotAllowZoneFills(True)
        zone.SetDoNotAllowTracks(False)
        zone.SetDoNotAllowVias(False)
        zone.SetDoNotAllowPads(False)
        zone.SetDoNotAllowFootprints(False)
        outline = zone.Outline()
        outline.NewOutline()
        half = k.FromMM(5)
        for dx, dy in [(-half,-half),(half,-half),(half,half),(-half,half)]:
            outline.Append(pos.x + dx, pos.y + dy)
        board.Add(zone)
        added[ref] = zone.m_Uuid.AsString()
    print('Refilling zones with KiCad...', flush=True)
    filler = k.ZONE_FILLER(board)
    assert filler.Fill(board.Zones()), 'Zone refill failed'
    assert k.SaveBoard(str(CANDIDATE), board, True)
    audit = verify(BEFORE, CANDIDATE, added)
    assert digest(MAIN) == original_hash
    for name, sha in untouched.items():
        assert digest(ROOT / name) == sha, f'Unexpected root file change: {name}'
    audit.update({'before_sha256': original_hash, 'candidate_sha256': digest(CANDIDATE),
                  'new_zone_ids': added, 'untouched_files': untouched, 'applied': False})
    AUDIT.write_text(json.dumps(audit, indent=2), encoding='utf-8')
    print(json.dumps(audit, indent=2), flush=True)


def apply():
    closed()
    audit = json.loads(AUDIT.read_text(encoding='utf-8'))
    assert not audit['applied']
    assert digest(MAIN) == audit['before_sha256'], 'Board changed since preparation'
    assert digest(CANDIDATE) == audit['candidate_sha256']
    for name, sha in audit['untouched_files'].items():
        assert digest(ROOT / name) == sha, f'Project file changed: {name}'
    verify(BEFORE, CANDIDATE, audit['new_zone_ids'])
    shutil.copy2(CANDIDATE, MAIN)
    assert digest(MAIN) == audit['candidate_sha256']
    audit['applied'] = True
    AUDIT.write_text(json.dumps(audit, indent=2), encoding='utf-8')
    print('Applied four verified 10mm copper-pour keepouts. Existing M3 footprints and schematic retained.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('action', choices=['prepare', 'apply'])
    args = parser.parse_args()
    globals()[args.action]()
