"""Remove only existing DRC-confirmed open-ended copper stubs."""
import pcbnew as k

STUBS={
    'f59949ae-a867-4e0b-885a-77c3a5398cd8',
    '7366855b-0763-4cd4-a28d-b1604704b9fc',
    '8e41cd0b-51ec-4a4c-a0ab-e98112387cff',
    '374b734b-6c79-40ed-a33e-fdc08c8c2cf9',
    'a3e7ed91-fd35-4374-a44a-1b51da4b439d',
    '79d8f561-d631-4fc6-8256-461783df2e87',
}
def apply(b):
    removed=[]
    for t in list(b.GetTracks()):
        if t.m_Uuid.AsString() in STUBS:
            assert not isinstance(t,k.PCB_VIA) and k.ToMM(t.GetLength())<1.3
            removed.append({'uuid':t.m_Uuid.AsString(),'net':t.GetNetname(),'length_mm':k.ToMM(t.GetLength())})
            b.Remove(t)
    return removed
