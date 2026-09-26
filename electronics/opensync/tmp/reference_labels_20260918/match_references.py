"""Copy CH1 reference-label geometry using KiCad's native API only."""
from pathlib import Path
import hashlib
import json
import sys

import pcbnew as k

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(__file__).resolve().parent
MAIN = ROOT / 'opensync.kicad_pcb'
BEFORE = WORK / 'before.kicad_pcb'
CANDIDATE = WORK / 'candidate.kicad_pcb'
sys.path.insert(0, r'C:\Users\Research\.codex\skills\kicad\scripts')
from sexp_parser import parse_file


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pair(vector):
    return [vector.x, vector.y]


def label_state(field):
    return {
        'text': field.GetText(), 'position': pair(field.GetPosition()),
        'angle': round(field.GetTextAngle().AsDegrees() % 360, 6),
        'size': pair(field.GetTextSize()), 'thickness': field.GetTextThickness(),
        'layer': field.GetLayer(), 'visible': field.IsVisible(),
        'keep_upright': field.IsKeepUpright(), 'mirrored': field.IsMirrored(),
        'bold': field.IsBold(), 'italic': field.IsItalic(),
        'hjustify': int(field.GetHorizJustify()),
        'vjustify': int(field.GetVertJustify()), 'font': field.GetFontName(),
    }


def mapped_footprints(board):
    sheets = {}
    for channel in range(1, 9):
        footprints = [f for f in board.GetFootprints()
                      if f.GetSheetname() == f'/Output Channel {channel}/']
        sheets[channel] = {f.GetPath().AsString().split('/')[-1]: f for f in footprints}
        assert len(footprints) == len(sheets[channel]) == 14, (channel, len(footprints))
        assert sheets[channel].keys() == sheets[1].keys(), channel
    return sheets


def masked_board(path, allowed):
    """Compare every token except intended reference positions/angles and writer ID."""
    tree = parse_file(str(path))
    children = []
    for item in tree[1:]:
        if not isinstance(item, list):
            children.append(item)
            continue
        if item[0] in ('generator', 'generator_version'):
            continue
        if item[0] == 'footprint':
            fields = [x for x in item if isinstance(x, list) and x
                      and x[0] == 'property' and x[1] == 'Reference']
            assert len(fields) == 1
            field = fields[0]
            if field[2] in allowed:
                positions = [x for x in field if isinstance(x, list) and x and x[0] == 'at']
                assert len(positions) == 1
                positions[0][:] = ['at', 'AUTHORIZED_REFERENCE_GEOMETRY']
        children.append(item)
    return sorted(json.dumps(x, ensure_ascii=False) for x in children)


def verify(path, report):
    allowed = set(report['targets'])
    assert masked_board(BEFORE, allowed) == masked_board(path, allowed), \
        'Unexpected change outside target reference-label positions/angles'
    board = k.LoadBoard(str(path))
    actual = {f.GetReference(): label_state(f.Reference()) for f in board.GetFootprints()}
    assert actual == report['expected_labels'], 'Saved reference-label state differs from expected'
    print('PASS: every non-reference-geometry board token preserved; all label states verified.')


def plan():
    assert BEFORE.exists(), 'Create an exact source backup first'
    assert digest(MAIN) == digest(BEFORE), 'Main board changed since backup'
    board = k.LoadBoard(str(BEFORE))
    sheets = mapped_footprints(board)
    expected = {f.GetReference(): label_state(f.Reference()) for f in board.GetFootprints()}
    targets, changes, mappings = [], [], []
    for channel in range(2, 9):
        for symbol, source in sheets[1].items():
            target = sheets[channel][symbol]
            assert source.GetFPIDAsString() == target.GetFPIDAsString()
            assert source.GetValue() == target.GetValue()
            assert abs(source.GetOrientationDegrees() - target.GetOrientationDegrees()) < 1e-6
            source_field, target_field = source.Reference(), target.Reference()
            old = label_state(target_field)
            source_state = label_state(source_field)
            for key in source_state:
                if key not in ('text', 'position', 'angle'):
                    assert old[key] == source_state[key], (target.GetReference(), 'style differs', key)
            position = target.GetPosition() + source_field.GetPosition() - source.GetPosition()
            delta = (source_state['angle'] - old['angle'] + 180) % 360 - 180
            if delta:
                target_field.Rotate(target_field.GetPosition(), k.EDA_ANGLE(delta, k.DEGREES_T))
            target_field.SetPosition(position)
            new = label_state(target_field)
            wanted = dict(old, position=pair(position), angle=source_state['angle'])
            assert new == wanted, (target.GetReference(), new, wanted)
            targets.append(target.GetReference())
            expected[target.GetReference()] = new
            mappings.append({'channel': channel, 'source': source.GetReference(),
                             'target': target.GetReference(), 'position_mm': [k.ToMM(v) for v in pair(position)],
                             'angle': new['angle']})
            if new != old:
                changes.append({'source': source.GetReference(), 'target': target.GetReference(),
                                'channel': channel, 'before': old, 'after': new})
    report = {'source_sha256': digest(BEFORE), 'targets': targets,
              'target_count': len(targets), 'changed_count': len(changes),
              'mappings': mappings, 'changes': changes, 'expected_labels': expected}
    assert k.SaveBoard(str(CANDIDATE), board), 'Candidate save failed'
    verify(CANDIDATE, report)
    (WORK / 'audit.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps({'target_count': len(targets), 'changed_count': len(changes),
                      'changes_by_channel': {str(i): [x['target'] for x in changes if x['channel'] == i]
                                             for i in range(2, 9)}}, indent=2))


if sys.argv[1:] == ['commit']:
    report = json.loads((WORK / 'audit.json').read_text(encoding='utf-8'))
    verify(CANDIDATE, report)
    assert digest(MAIN) == report['source_sha256'], 'Main PCB changed; refusing to overwrite'
    board = k.LoadBoard(str(CANDIDATE))
    assert k.SaveBoard(str(MAIN), board), 'Native save failed'
    verify(MAIN, report)
    print('Saved reference-label-only edit:', MAIN)
elif sys.argv[1:] == ['verify']:
    verify(MAIN, json.loads((WORK / 'audit.json').read_text(encoding='utf-8')))
else:
    assert not sys.argv[1:]
    plan()
