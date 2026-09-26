"""Increase two existing CH5 paired-via center spacings to 0.6 mm."""
import pcbnew as k

def apply(board):
    net='/Output Channel 5/VOUT'
    moves=[((128.3,103.5),(128.3,103.6)),((130.8,123.8),(130.7,123.8))]
    report=[]
    def vec(xy): return k.VECTOR2I(round(xy[0]*1_000_000),round(xy[1]*1_000_000))
    for old,new in moves:
        oldp,newp=vec(old),vec(new)
        vias=[t for t in board.GetTracks() if isinstance(t,k.PCB_VIA) and t.GetNetname()==net and t.GetPosition()==oldp]
        if not vias:
            if any(isinstance(t,k.PCB_VIA) and t.GetNetname()==net and t.GetPosition()==newp for t in board.GetTracks()):
                continue
            raise RuntimeError('Expected original channel-5 via not found at '+repr(old))
        if len(vias)!=1: raise RuntimeError('Ambiguous original channel-5 via at '+repr(old))
        changes=0
        for t in board.GetTracks():
            if isinstance(t,k.PCB_VIA) or t.GetNetname()!=net: continue
            if t.GetStart()==oldp: t.SetStart(newp); changes+=1
            if t.GetEnd()==oldp: t.SetEnd(newp); changes+=1
        vias[0].SetPosition(newp)
        report.append({'from':old,'to':new,'attached_endpoints_updated':changes})
    # Join the spare source via on both routing layers, matching the duplicated channels.
    a,b=vec((128.3,103.0)),vec((128.3,103.6))
    if not any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==net and t.GetLayer()==k.F_Cu and ((t.GetStart()==a and t.GetEnd()==b) or (t.GetStart()==b and t.GetEnd()==a)) for t in board.GetTracks()):
        t=k.PCB_TRACK(board); t.SetStart(a); t.SetEnd(b); t.SetWidth(k.FromMM(.3)); t.SetLayer(k.F_Cu); t.SetNetCode(board.FindNet(net).GetNetCode()); board.Add(t)
        report.append({'top_pair_link_added':True})
    return report

if __name__=='__main__':
    import sys
    b=k.LoadBoard(sys.argv[1]); print(apply(b)); k.ZONE_FILLER(b).Fill(b.Zones()); k.SaveBoard(sys.argv[2],b)
