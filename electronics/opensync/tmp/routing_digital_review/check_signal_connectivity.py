import pcbnew as p
import collections

b = p.LoadBoard('opensync.kicad_pcb')
c = b.GetConnectivity()
nets = collections.defaultdict(list)
for f in b.GetFootprints():
    for pad in f.Pads():
        nets[pad.GetNetname()].append(pad)

def connected_pads(start):
    stack = [start]
    seen = set()
    pads = set()
    while stack:
        item = stack.pop()
        uid = item.m_Uuid.AsString()
        if uid in seen:
            continue
        seen.add(uid)
        if isinstance(item, p.PAD):
            pads.add(item.GetParentFootprint().GetReference()+'.'+item.GetNumber())
        stack.extend(c.GetConnectedPads(item))
        stack.extend(c.GetConnectedTracks(item))
    return pads

for name in sorted(nets):
    if not any(t in name for t in ('/IN_', 'USB', 'QSPI', '/XIN', '/XOUT', 'Net-(C3', 'Net-(U29', 'Net-(U30', 'Net-(U33', 'Net-(U35', 'SWCLK', '/SWD', '/RUN')):
        continue
    allpads = {pad.GetParentFootprint().GetReference()+'.'+pad.GetNumber() for pad in nets[name]}
    components = set()
    for pad in nets[name]:
        components.add(tuple(sorted(connected_pads(pad))))
    print(name, 'fully-connected' if len(components) == 1 else 'INCOMPLETE', list(components))

