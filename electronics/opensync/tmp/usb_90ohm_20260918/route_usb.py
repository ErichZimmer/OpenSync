"""USB-only native KiCad routing, with unchanged non-USB design invariants."""
from pathlib import Path
import hashlib
import json
import math
import sys

import pcbnew as k

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(__file__).resolve().parent
MAIN = ROOT / 'opensync.kicad_pcb'
BEFORE = WORK / 'before.kicad_pcb'
CANDIDATE = WORK / 'candidate.kicad_pcb'
PRO = ROOT / 'opensync.kicad_pro'
WIDTH, GAP = .158, .200
PITCH = WIDTH + GAP
RAW_P, RAW_N = 'Net-(D14-I{slash}O_1)', 'Net-(D14-I{slash}O_2)'
MID_P, MID_N = '/Microcontroller Circuitry/USB_D+', '/Microcontroller Circuitry/USB_D-'
END_P, END_N = 'Net-(U1-USB_DP)', 'Net-(U1-USB_DM)'
USB = {RAW_P, RAW_N, MID_P, MID_N, END_P, END_N}
sys.path.insert(0, r'C:\Users\Research\.codex\skills\kicad\scripts')
from sexp_parser import parse_file


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def point(x, y):
    return k.VECTOR2I(k.FromMM(round(x, 6)), k.FromMM(round(y, 6)))


def xy(vector):
    return [round(k.ToMM(vector.x), 6), round(k.ToMM(vector.y), 6)]


def immutable(path):
    """Full non-USB board comparison; zone definitions remain immutable."""
    tree = parse_file(str(path))
    children = []
    for item in tree[1:]:
        if not isinstance(item, list):
            children.append(item)
            continue
        if item[0] in ('generator', 'generator_version'):
            continue
        if item[0] in ('segment', 'arc', 'via'):
            net = next((x[1] for x in item if isinstance(x, list) and x and x[0] == 'net'), None)
            if net in {n.replace('{slash}', '/') for n in USB}:
                assert item[0] == 'segment', 'Unexpected USB via or arc'
                continue
        if item[0] == 'zone':
            item = [x for x in item if not (isinstance(x, list) and x
                    and x[0] in ('filled_polygon', 'fill_segments', 'fill_polygon', 'island'))]
        children.append(item)
    return sorted(json.dumps(x, ensure_ascii=False) for x in children)


def usb_geometry(board):
    return sorted((t.GetNetname(), tuple(xy(t.GetStart())), tuple(xy(t.GetEnd())),
                   round(k.ToMM(t.GetWidth()), 6), t.GetLayerName())
                  for t in board.GetTracks() if t.GetNetname() in USB)


def check(path):
    assert immutable(BEFORE) == immutable(path), 'Non-USB design geometry changed'
    board = k.LoadBoard(str(path))
    tracks = [t for t in board.GetTracks() if t.GetNetname() in USB]
    assert tracks and all(not isinstance(t, k.PCB_VIA) for t in tracks)
    assert all(t.GetWidth() == k.FromMM(WIDTH) and t.GetLayer() == k.F_Cu for t in tracks)
    assert {t.GetNetname() for t in tracks} == USB
    print('PASS: all USB tracks are 0.158 mm on F.Cu; no USB vias; all other design geometry preserved.')
    return board


def main():
    assert digest(MAIN) == digest(BEFORE), 'Board changed since backup'
    assert digest(PRO) == digest(WORK / 'before.kicad_pro'), 'Project settings changed since backup'
    board = k.LoadBoard(str(BEFORE))
    removed = [t for t in board.GetTracks() if t.GetNetname() in USB]
    assert len(removed) == 36, len(removed)
    assert all(not isinstance(t, k.PCB_VIA) for t in removed)
    for t in removed:
        board.Remove(t)
    routes = []

    def route(net, points, purpose):
        routes.append({'net': net, 'points': [[round(x, 6), round(y, 6)] for x, y in points], 'purpose': purpose})
        for a, b in zip(points, points[1:]):
            if math.dist(a, b) < 1e-7:
                continue
            track = k.PCB_TRACK(board)
            track.SetStart(point(*a))
            track.SetEnd(point(*b))
            track.SetWidth(k.FromMM(WIDTH))
            track.SetLayer(k.F_Cu)
            track.SetNetCode(board.FindNet(net).GetNetCode())
            board.Add(track)

    # Connector pin escape is local; the long raw pair is exactly PITCH apart.
    left, right = 65 - PITCH / 2, 65 + PITCH / 2
    fan_y = 74.05 - (left - 64.5)
    route(RAW_P, [(65.25, 61.71), (65.09, 61.71), (63.9, 62.9),
          (63.9, 64.2), (left, 64.2 + left - 63.9),
          (left, 71.95), (left, fan_y), (64.5, 74.05), (64.5, 74.65)],
          'Connector D+ to choke, with coupled vertical trunk')
    route(RAW_N, [(65.25, 63.71), (65.25, 64.65),
          (right, 64.65 + 65.25 - right), (right, 71.95),
          (right, fan_y), (65.5, 74.05), (65.5, 74.65)],
          'Connector D- to choke, with coupled vertical trunk')
    route(RAW_P, [(63.75, 71.95), (left, 71.95)], 'Short connection to D14 protection pad 3')
    route(RAW_N, [(66.25, 71.95), (right, 71.95)], 'Short connection to D14 protection pad 4')

    # Preserve available main corridor, correct pitch on the straight and bend.
    route(MID_P, [(64.5, 77.35), (64.5, 78.4), (65.8, 78.4),
          (67.2, 77.0), (84.55, 77.0), (85, 77.45), (85, 78.69)],
          'Choke to R10; long controlled-impedance pair')
    route(MID_N, [(65.5, 77.35), (65.5 + PITCH + .35, 77.0 - PITCH),
          (84.55 + PITCH * (math.sqrt(2) - 1), 77.0 - PITCH),
          (86, 78.45 - PITCH * math.sqrt(2)), (86, 78.69)],
          'Choke to R11; constant perpendicular spacing at paired bend')

    # Keep both traces left of the 1V1-via L2 antipads, then fan into the MCU.
    route(END_N, [(86, 79.71), (86, 79.9), (85.65, 80.25),
          (85.65, 81.4), (86.8, 82.55), (86.8, 83.1035)],
          'R11 to MCU USB_DM')
    route(END_P, [(85, 79.71), (85, 80), (85.65 - PITCH, 80.65 - PITCH),
          (85.65 - PITCH, 81.4 + PITCH * (math.sqrt(2) - 1)),
          (86.4, 82.15 + PITCH * math.sqrt(2)), (86.4, 83.1035)],
          'R10 to MCU USB_DP, coupled vertical and diagonal sections')
    board.BuildConnectivity()
    k.ZONE_FILLER(board).Fill(board.Zones())
    assert k.SaveBoard(str(CANDIDATE), board, True), 'Native candidate save failed'
    assert digest(PRO) == digest(WORK / 'before.kicad_pro'), 'Settings side effect'
    check(CANDIDATE)
    report = {
        'before_sha256': digest(BEFORE), 'project_sha256': digest(PRO),
        'width_mm': WIDTH, 'edge_gap_mm': GAP, 'center_pitch_mm': PITCH,
        'calculator_nominal_ohm': 89.9522758704,
        'calculator_model': 'DiffEdgeCoupledCoatedMicrostrip1B',
        'calculator_parameters_mil': {'H1': .0994 / .0254, 'Er1': 4.1,
            'W1': WIDTH / .0254, 'W2': WIDTH / .0254 - .5,
            'S1': GAP / .0254, 'T1': 1.6, 'C1': 1., 'C2': .6, 'C3': 1., 'CEr': 3.8},
        'source': 'https://jlcpcb.com/pcb-impedance-calculator',
        'routes': routes, 'removed_tracks': len(removed),
        'new_tracks': sum(len(r['points']) - 1 for r in routes),
        'scope': 'Only USB signal tracks and dependent zone fills; no component, pad, net, silk or non-USB track changes',
        'geometry': usb_geometry(board),
    }
    (WORK / 'audit.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps({'removed': len(removed), 'added': report['new_tracks'], 'candidate': str(CANDIDATE)}))


if sys.argv[1:] == ['commit']:
    report = json.loads((WORK / 'audit.json').read_text(encoding='utf-8'))
    assert digest(MAIN) == report['before_sha256'], 'Source changed; stopping'
    assert digest(PRO) == report['project_sha256'], 'Project changed; stopping'
    check(CANDIDATE)
    board = k.LoadBoard(str(CANDIDATE))
    assert usb_geometry(board) == [tuple([x[0], tuple(x[1]), tuple(x[2]), x[3], x[4]]) for x in report['geometry']]
    assert k.SaveBoard(str(MAIN), board, True), 'Native final save failed'
    assert digest(PRO) == report['project_sha256'], 'Project settings changed'
    print('Saved verified USB-only PCB geometry with project settings untouched.')
elif sys.argv[1:] == ['verify']:
    check(MAIN)
    assert digest(PRO) == digest(WORK / 'before.kicad_pro'), 'Project settings changed'
else:
    assert not sys.argv[1:]
    main()
