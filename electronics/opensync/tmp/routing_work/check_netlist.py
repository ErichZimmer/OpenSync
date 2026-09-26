"""Independent pin-by-pin comparison with KiCad's exported schematic netlist."""
import sys,json
from pathlib import Path
import xml.etree.ElementTree as ET
import pcbnew as k

def check(board_path,xml_path):
    root=ET.parse(xml_path).getroot();expected={}
    for n in root.findall('./nets/net'):
        for p in n.findall('node'):expected[p.attrib['ref'],p.attrib['pin']]=n.attrib['name']
    b=k.LoadBoard(str(board_path));mismatch=[];checked=0
    for f in b.GetFootprints():
        for p in f.Pads():
            key=f.GetReference(),p.GetNumber()
            if not p.GetNumber() or not p.GetNetname():continue
            checked+=1
            # pcbnew stores literal slashes in auto-generated pin names escaped.
            actual=p.GetNetname().replace('{slash}','/')
            if expected.get(key)!=actual:mismatch.append({'reference':key[0],'pad':key[1],'pcb':actual,'schematic':expected.get(key)})
    report={'checked_connected_pads':checked,'mismatches':mismatch}
    print(json.dumps(report,indent=2));return report

if __name__=='__main__':
    r=check(sys.argv[1],sys.argv[2])
    if len(sys.argv)>3:Path(sys.argv[3]).write_text(json.dumps(r,indent=2))
    assert not r['mismatches'],'PCB/schematic pin net mismatch'
