"""Replicate CH5's selector VOUT changes with the native KiCad API only."""
from pathlib import Path
import hashlib
import json
import re
import sys
import pcbnew as k

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(__file__).resolve().parent
SOURCE = WORK / 'before.kicad_pcb'
CANDIDATE = WORK / 'candidate.kicad_pcb'
MAIN = ROOT / 'opensync.kicad_pcb'
sys.path.insert(0, str(ROOT / 'tmp/routing_channel_review'))
from route_channels import Collision

def uid(item): return item.m_Uuid.AsString()
def vec(q): return k.VECTOR2I(round(q[0]*k.FromMM(1)), round(q[1]*k.FromMM(1)))
def xy(q): return (round(k.ToMM(q.x), 6), round(k.ToMM(q.y), 6))
def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def tracks(board):
    result = {}
    for t in board.GetTracks():
        v = isinstance(t, k.PCB_VIA)
        result[uid(t)] = {
            'type': type(t).__name__, 'net': t.GetNetname(),
            'start': xy(t.GetStart()), 'end': xy(t.GetEnd()),
            'layers': t.GetLayerSet().FmtBin(), 'locked': t.IsLocked(),
            'width': t.GetWidth(k.F_Cu) if v else t.GetWidth(),
            'drill': t.GetDrillValue() if v else None,
        }
    return result

def immutable_blocks(path):
    # Read-only token comparison of footprint/pad/model and other non-copper blocks.
    tokens = re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+', path.read_text(encoding='utf-8'))
    depth = 0
    blocks = []
    start = None
    for i, token in enumerate(tokens):
        if token == '(':
            if depth == 1: start = i
            depth += 1
        elif token == ')':
            depth -= 1
            if depth == 1:
                block = tokens[start:i+1]
                if block[1] not in {'segment', 'arc', 'via', 'zone', 'generator', 'generator_version'}:
                    blocks.append(tuple(block))
    return sorted(blocks)

def zone_settings(path):
    board = k.LoadBoard(str(path))
    return {uid(z): (
        z.GetNetname(), z.GetLayerSet().FmtBin(), z.GetZoneName(),
        z.GetAssignedPriority(), z.GetLocalClearance(), z.GetMinThickness(),
        z.GetIsRuleArea(), z.GetDoNotAllowTracks(), z.GetDoNotAllowVias(),
        z.GetDoNotAllowPads(), z.GetDoNotAllowZoneFills(),
        tuple(tuple(xy(z.Outline().COutline(i).CPoint(j))
            for j in range(z.Outline().COutline(i).PointCount()))
            for i in range(z.Outline().OutlineCount()))
    ) for z in board.Zones()}

def verify(path):
    report = json.loads((WORK / 'edit-audit.json').read_text())
    actual = json.loads(json.dumps(tracks(k.LoadBoard(str(path)))))
    assert actual == report['expected_tracks'], 'Track/via state differs from approved edits'
    assert immutable_blocks(SOURCE) == immutable_blocks(path), 'Non-copper board data changed'
    assert zone_settings(SOURCE) == zone_settings(path), 'Zone definition changed'
    print('PASS: exact allowed track/via edits; footprints, pads, models and zone definitions preserved')

if sys.argv[1:] == ['verify']:
    verify(MAIN)
    raise SystemExit
if sys.argv[1:] == ['commit']:
    verify(CANDIDATE)
    drc = json.loads((WORK / 'candidate-drc.json').read_text())
    assert not drc['unconnected_items'], 'Candidate has unconnected items'
    assert not [v for v in drc['violations'] if v['severity'] == 'error'], 'Candidate has DRC errors'
    assert digest(MAIN) == digest(SOURCE), 'Main board changed since snapshot; stop rather than overwrite'
    board = k.LoadBoard(str(CANDIDATE))
    assert k.SaveBoard(str(MAIN), board), 'Native save failed'
    verify(MAIN)
    print('Saved', MAIN)
    raise SystemExit

assert not sys.argv[1:]
board = k.LoadBoard(str(SOURCE))
before = tracks(board)
changed, removed, added, summary = set(), set(), set(), []
removed_items = []  # Keep native wrappers alive for the editing session.

def find_segment(net, layer, a, b):
    expected = {xy(vec(a)), xy(vec(b))}
    found = [t for t in board.GetTracks() if not isinstance(t, k.PCB_VIA)
        and t.GetNetname() == net and t.GetLayer() == layer
        and {xy(t.GetStart()), xy(t.GetEnd())} == expected]
    assert len(found) == 1, (net, a, b, len(found))
    return found[0]

def remove_segment(net, layer, a, b):
    t = find_segment(net, layer, a, b)
    removed.add(uid(t)); board.Remove(t); removed_items.append(t)

template = board.FindFootprintByReference('U11')
assert xy(template.GetPosition()) == (129.3, 93.0)
assert find_segment('/Output Channel 5/VOUT', k.F_Cu, (128.56,93.25), (130.04,93.25)).GetWidth() == k.FromMM(.3)
targets = [(1,'U3',0,-21),(2,'U5',16,-21),(3,'U7',32,-21),
    (4,'U9',48,-21),(6,'U13',16,0),(7,'U15',32,0),(8,'U17',48,0)]
for channel, ref, dx, dy in targets:
    shift = lambda p: (round(p[0]+dx,6), round(p[1]+dy,6))
    name = f'/Output Channel {channel}/VOUT'
    fp = board.FindFootprintByReference(ref)
    assert xy(fp.GetPosition()) == shift((129.3,93))
    assert fp.GetOrientationDegrees() == template.GetOrientationDegrees()
    bridge = find_segment(name, k.F_Cu, shift((128.56,93.25)), shift((130.04,93.25)))
    assert bridge.GetWidth() == k.FromMM(.2)
    assert Collision(board,k.F_Cu,name,.3,.18).clear(xy(bridge.GetStart()),xy(bridge.GetEnd()))
    bridge.SetWidth(k.FromMM(.3)); changed.add(uid(bridge))
    for old, new in [((128.3,103),(129.3,101)),((128.3,103.6),(129.3,101.6))]:
        found = [t for t in board.GetTracks() if isinstance(t,k.PCB_VIA)
            and t.GetNetname() == name and xy(t.GetPosition()) == shift(old)]
        assert len(found) == 1
        v = found[0]
        assert v.GetWidth(k.F_Cu) == k.FromMM(.6) and v.GetDrillValue() == k.FromMM(.3)
        for layer in [k.F_Cu,k.In1_Cu,k.In2_Cu,k.B_Cu]:
            assert Collision(board,layer,name,.6,.18).clear(shift(new),shift(new))
        v.SetPosition(vec(shift(new))); changed.add(uid(v))
    remove_segment(name,k.F_Cu,shift((129.275,103)),shift((128.3,103)))
    remove_segment(name,k.F_Cu,shift((128.3,103)),shift((128.3,103.6)))
    remove_segment(name,k.In2_Cu,shift((128.3,103)),shift((128.3,103.6)))
    old = shift((128.3,103))
    exits = [t for t in board.GetTracks() if not isinstance(t,k.PCB_VIA)
        and t.GetNetname() == name and t.GetLayer() == k.In2_Cu
        and old in (xy(t.GetStart()),xy(t.GetEnd()))]
    assert len(exits) == 1
    exit_track = exits[0]
    assert exit_track.GetWidth() == k.FromMM(.4)
    join = shift((129.3,102))
    end = xy(exit_track.GetEnd()) if xy(exit_track.GetStart()) == old else xy(exit_track.GetStart())
    col = Collision(board,k.In2_Cu,name,.4,.18)
    assert col.clear(join,end) and col.clear(shift((129.3,101)),join)
    if xy(exit_track.GetStart()) == old: exit_track.SetStart(vec(join))
    else: exit_track.SetEnd(vec(join))
    changed.add(uid(exit_track))
    stem = k.PCB_TRACK(board)
    stem.SetStart(vec(shift((129.3,101)))); stem.SetEnd(vec(join))
    stem.SetWidth(k.FromMM(.4)); stem.SetLayer(k.In2_Cu)
    stem.SetNetCode(board.FindNet(name).GetNetCode()); board.Add(stem); added.add(uid(stem))
    summary.append({'channel':channel,'selector':ref,'vias':[shift((129.3,101)),shift((129.3,101.6))],'pin_bridge_width':.3})

after = tracks(board)
assert set(before)-set(after) == removed and set(after)-set(before) == added
assert {u for u in set(before)&set(after) if before[u] != after[u]} == changed
assert len(changed) == 28 and len(removed) == 21 and len(added) == 7
assert all(before[u] == after[u] for u in before if before[u]['net'] == '/Output Channel 5/VOUT')
assert k.SaveBoard(str(CANDIDATE),board)
(WORK/'edit-audit.json').write_text(json.dumps({'source_sha256':digest(SOURCE),
    'summary':summary,'changed':sorted(changed),'removed':sorted(removed),'added':sorted(added),
    'expected_tracks':after},indent=2))
verify(CANDIDATE)
print(json.dumps(summary,indent=2))
