"""Copy D10's 3D settings with KiCad's API; verify all other PCB data."""
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
CANDIDATE = WORK / 'candidate.kicad_pcb'
AUDIT = WORK / 'audit.json'
TARGETS = {f'D{i}' for i in range(2, 10)} | {'D12', 'D13'}
sys.path.insert(0, r'C:\Users\Research\.codex\skills\kicad\scripts')
from sexp_parser import parse_file


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def assert_editor_closed():
    result = subprocess.run(
        ['tasklist', '/FI', 'IMAGENAME eq pcbnew.exe', '/FO', 'CSV', '/NH'],
        check=True, capture_output=True, text=True,
    )
    assert 'pcbnew.exe' not in result.stdout.lower(), 'PCB editor opened: stop to protect live edits'
    assert not list(ROOT.glob('*.lck')), 'KiCad lock file detected'


def model_info(model):
    return {
        'filename': model.m_Filename,
        'offset': [model.m_Offset.x, model.m_Offset.y, model.m_Offset.z],
        'rotation': [model.m_Rotation.x, model.m_Rotation.y, model.m_Rotation.z],
        'scale': [model.m_Scale.x, model.m_Scale.y, model.m_Scale.z],
        'show': model.m_Show,
        'opacity': model.m_Opacity,
    }


def reference(node):
    return next(child[2] for child in node if isinstance(child, list)
                and child[:2] == ['property', 'Reference'])


def model_nodes(tree):
    return {reference(node): [child for child in node if isinstance(child, list)
                             and child[:1] == ['model']]
            for node in tree if isinstance(node, list) and node[:1] == ['footprint']}


def without_target_models(tree):
    result = copy.deepcopy(tree)
    for node in result:
        if isinstance(node, list) and node[:1] == ['footprint'] and reference(node) in TARGETS:
            node[:] = [child for child in node
                       if not (isinstance(child, list) and child[:1] == ['model'])]
    return result


def verify(before, after):
    left, right = parse_file(str(before)), parse_file(str(after))
    assert without_target_models(left) == without_target_models(right), 'Non-target PCB data changed'
    lm, rm = model_nodes(left), model_nodes(right)
    changed = {ref for ref in lm if lm[ref] != rm[ref]}
    assert changed == TARGETS, f'Unexpected changed model set: {changed}'
    assert lm['D10'] == rm['D10'], 'D10 changed'
    for ref in TARGETS:
        assert rm[ref] == rm['D10'], f'Models do not exactly match D10: {ref}'
    return sorted(changed, key=lambda ref: int(ref[1:]))


def prepare():
    assert_editor_closed()
    assert not BEFORE.exists() and not CANDIDATE.exists(), 'Existing protected work: do not overwrite'
    model_file = ROOT / 'library' / 'LED_replacement_cad' / 'LED_A67F.step'
    assert model_file.is_file(), 'D10 replacement STEP is missing'
    untouched = {str(path.relative_to(ROOT)): digest(path) for path in ROOT.iterdir()
                 if path.is_file() and path != MAIN}
    untouched[str(model_file.relative_to(ROOT))] = digest(model_file)
    board_hash = digest(MAIN)
    shutil.copy2(MAIN, BEFORE)
    board = k.LoadBoard(str(MAIN))
    fps = {fp.GetReference(): fp for fp in board.GetFootprints()}
    source = fps['D10']
    source_models = [model_info(model) for model in source.Models()]
    assert len(source_models) == 2
    assert source_models[0]['show'] is False
    assert source_models[1]['filename'] == '${KIPRJMOD}/library/LED_replacement_cad/LED_A67F.step'
    matches = {ref for ref, fp in fps.items() if fp.GetFPID() == source.GetFPID()}
    assert matches == TARGETS | {'D10'}, f'Unexpected same-footprint set: {matches}'
    for ref in sorted(TARGETS):
        fp = fps[ref]
        assert fp.GetOrientationDegrees() == source.GetOrientationDegrees()
        fp.Models().clear()
        for spec in source_models:
            model = k.FP_3DMODEL()
            model.m_Filename = spec['filename']
            model.m_Show = spec['show']
            model.m_Opacity = spec['opacity']
            for field, key in [('m_Offset', 'offset'), ('m_Rotation', 'rotation'), ('m_Scale', 'scale')]:
                vector = getattr(model, field)
                vector.x, vector.y, vector.z = spec[key]
            fp.Add3DModel(model)
    # Prevent KiCad from writing a project-settings file as a SaveBoard side effect.
    assert k.SaveBoard(str(CANDIDATE), board, True), 'Native candidate save failed'
    changed = verify(BEFORE, CANDIDATE)
    assert digest(MAIN) == board_hash, 'User changed board during preparation'
    for name, sha in untouched.items():
        assert digest(ROOT / name) == sha, f'Other project file changed: {name}'
    audit = {
        'before_sha256': board_hash,
        'candidate_sha256': digest(CANDIDATE),
        'updated_refs': changed,
        'source_ref': 'D10',
        'source_models': source_models,
        'other_project_hashes': untouched,
        'all_non_model_board_data_identical': True,
        'applied': False,
    }
    AUDIT.write_text(json.dumps(audit, indent=2), encoding='utf-8')
    print(json.dumps({key: value for key, value in audit.items() if key != 'other_project_hashes'}, indent=2))


def apply():
    assert_editor_closed()
    audit = json.loads(AUDIT.read_text(encoding='utf-8'))
    assert not audit['applied'], 'Already applied'
    assert digest(MAIN) == audit['before_sha256'], 'Live board changed: stop instead of overwriting'
    assert digest(CANDIDATE) == audit['candidate_sha256'], 'Candidate changed since verification'
    verify(BEFORE, CANDIDATE)
    for name, sha in audit['other_project_hashes'].items():
        assert digest(ROOT / name) == sha, f'Other project file changed: {name}'
    board = k.LoadBoard(str(CANDIDATE))
    assert digest(MAIN) == audit['before_sha256'], 'Live board changed during candidate load'
    assert_editor_closed()
    assert k.SaveBoard(str(MAIN), board, True), 'Native final save failed'
    verify(BEFORE, MAIN)
    assert digest(MAIN) == audit['candidate_sha256'], 'Final save does not exactly match candidate'
    for name, sha in audit['other_project_hashes'].items():
        assert digest(ROOT / name) == sha, f'Non-PCB file changed: {name}'
    audit['applied'] = True
    audit['final_sha256'] = digest(MAIN)
    AUDIT.write_text(json.dumps(audit, indent=2), encoding='utf-8')
    print(json.dumps({'applied': True, 'updated_refs': audit['updated_refs'],
                      'other_board_data_unchanged': True, 'project_settings_unchanged': True,
                      'backup': str(BEFORE)}, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('mode', choices=['prepare', 'apply'])
    args = parser.parse_args()
    prepare() if args.mode == 'prepare' else apply()
