"""Prune two redundant reserved LED egress detours, preserving all terminals."""
from pathlib import Path
import pcbnew as p
from route_channels import Collision,xy,vec

def apply(board):
    specs=[
        ('LED_D', [((73.2,81.0),(73.2,67.9)),((73.2,67.9),(73.2,82.8))],
         [(73.2,81.0),(73.2,82.8)]),
        ('LED_E', [((72.0,82.0),(72.0,67.2)),((72.0,67.2),(72.2,67.4)),((72.2,67.4),(72.2,82.9))],
         [(72.0,82.0),(72.2,82.2),(72.2,82.9)]),
    ]
    changes=[]
    for name,old,new in specs:
        nn='/Indicator Panel/'+name
        checker=Collision(board,p.In2_Cu,nn,width=.2,clearance=.18)
        if not all(checker.clear(a,z) for a,z in zip(new,new[1:])):
            raise RuntimeError('Native collision check rejected LED shortcut '+name)
        matched=[]
        for a,z in old:
            ts=[t for t in board.GetTracks() if not isinstance(t,p.PCB_VIA) and t.GetNetname()==nn and t.GetLayer()==p.In2_Cu and
                ((xy(t.GetStart())==a and xy(t.GetEnd())==z) or (xy(t.GetEnd())==a and xy(t.GetStart())==z))]
            if len(ts)!=1:raise RuntimeError('Expected exact original LED detour segment '+str((name,a,z,len(ts))))
            matched.extend(ts)
        for t in matched:board.Remove(t)
        for a,z in zip(new,new[1:]):
            t=p.PCB_TRACK(board);t.SetNetCode(board.FindNet(nn).GetNetCode());t.SetLayer(p.In2_Cu)
            t.SetWidth(p.FromMM(.2));t.SetStart(vec(a));t.SetEnd(vec(z));board.Add(t)
        changes.append({'net':nn,'removed_segments':len(matched),'new_path':new})
    board.BuildConnectivity()
    return changes

if __name__=='__main__':
    import json,shutil,subprocess,sys
    folder=Path(__file__).resolve().parent;source=folder.parent/'routing_work'/'integrated.kicad_pcb'
    board=p.LoadBoard(str(source))
    pruned=apply(board)
    # A separate native process avoids KiCad SWIG ownership/cache issues after
    # removing tracks and before filling zones. No main board is saved here.
    temporary=folder/'led-pruned.kicad_pcb';p.SaveBoard(str(temporary),board)
    shutil.copy2(source.with_suffix('.kicad_pro'),temporary.with_suffix('.kicad_pro'))
    print(json.dumps({'pruned':pruned},indent=2),flush=True)
    out=folder/'led-final.kicad_pcb'
    subprocess.run([sys.executable,'-B',str(folder/'stitch_led_returns.py'),str(temporary),str(out)],check=True)
