"""Commit only a clean, audited native KiCad candidate to the project PCB."""
import hashlib,json
from pathlib import Path
import pcbnew as k
ROOT=Path(__file__).resolve().parents[2]
work=ROOT/'tmp/routing_work'
source=ROOT/'opensync.kicad_pcb'
baseline=work/'baseline.kicad_pcb'
assert hashlib.sha256(source.read_bytes()).digest()==hashlib.sha256(baseline.read_bytes()).digest(),'Main PCB changed while routing; refusing to overwrite'
drc=json.loads((work/'integrated-drc.json').read_text())
errors=[v for v in drc['violations'] if v['severity']=='error']
assert not errors and not drc['unconnected_items'],'Candidate is not electrically DRC clean'
invariants=json.loads((work/'invariants.json').read_text())
assert invariants['pad_definitions_unchanged'] and invariants['outline_unchanged']
netcheck=json.loads((work/'netlist-check.json').read_text())
assert not netcheck['mismatches']
b=k.LoadBoard(str(work/'integrated.kicad_pcb'))
k.SaveBoard(str(source),b)
print('Saved verified candidate using native KiCad API:',source)
