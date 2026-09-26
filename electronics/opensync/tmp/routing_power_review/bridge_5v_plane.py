"""Restore the +5V plane across the LED control fan-out using a short F.Cu strap."""
import pcbnew as k

def apply(board):
    net=board.FindNet('+5V').GetNetCode()
    def vec(p): return k.VECTOR2I(round(p[0]*1_000_000),round(p[1]*1_000_000))
    pairs=[((137.4,74.3),(138.2,74.3)),((137.4,78.5),(138.2,78.5)),((137.8,74.3),(137.8,78.5))]
    created={'tracks':0,'vias':0}
    for a,b in pairs:
        av,bv=vec(a),vec(b)
        if any(not isinstance(t,k.PCB_VIA) and t.GetNetCode()==net and t.GetLayer()==k.F_Cu and ((t.GetStart()==av and t.GetEnd()==bv) or (t.GetStart()==bv and t.GetEnd()==av)) for t in board.GetTracks()): continue
        t=k.PCB_TRACK(board); t.SetStart(av); t.SetEnd(bv); t.SetLayer(k.F_Cu); t.SetNetCode(net); t.SetWidth(k.FromMM(.8)); board.Add(t); created['tracks']+=1
    for p in [(137.4,74.3),(138.2,74.3),(137.4,78.5),(138.2,78.5)]:
        vp=vec(p)
        if any(isinstance(v,k.PCB_VIA) and v.GetNetCode()==net and v.GetPosition()==vp for v in board.GetTracks()): continue
        v=k.PCB_VIA(board); v.SetPosition(vp); v.SetWidth(k.FromMM(.6)); v.SetDrill(k.FromMM(.3)); v.SetViaType(k.VIATYPE_THROUGH); v.SetLayerPair(k.F_Cu,k.B_Cu); v.SetNetCode(net); board.Add(v); created['vias']+=1
    return created

if __name__=='__main__':
    import sys
    b=k.LoadBoard(sys.argv[1]); print(apply(b)); k.ZONE_FILLER(b).Fill(b.Zones()); k.SaveBoard(sys.argv[2],b)
