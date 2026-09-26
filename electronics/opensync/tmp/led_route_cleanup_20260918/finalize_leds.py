from organize_leds import *
import shutil

b=k.LoadBoard(str(WORK/'agent-entry-vias.kicad_pcb'))
retired=[]
for name,pt in [('LED_G',(92.6,70.5)),('LED_ACTIVE',(92.6,66.9))]:
    matches=[t for t in b.GetTracks() if isinstance(t,k.PCB_VIA) and t.GetNetname()==net(name) and xy(t.GetPosition())==pt]
    assert len(matches)==1
    v=matches[0]
    assert not any(not isinstance(t,k.PCB_VIA) and t.GetNetname()==net(name) and t.GetLayer()==k.F_Cu
        and v.GetEffectiveShape(k.F_Cu).Collide(t.GetEffectiveShape(k.F_Cu),0) for t in b.GetTracks())
    b.Remove(v);retired.append(v)

merged=0
while True:
    vertices={}
    for t in b.GetTracks():
        if isinstance(t,k.PCB_VIA) or t.GetNetname() not in NAMES:continue
        for pos in (xy(t.GetStart()),xy(t.GetEnd())):
            vertices.setdefault((t.GetNetname(),t.GetLayer(),pos),[]).append(t)
    found=False
    for (nn,layer,pos),ts in vertices.items():
        if len(ts)!=2:continue
        a,z=ts
        if a.GetWidth()!=z.GetWidth() or a.IsLocked()!=z.IsLocked():continue
        pa=xy(a.GetStart()) if xy(a.GetEnd())==pos else xy(a.GetEnd())
        pz=xy(z.GetStart()) if xy(z.GetEnd())==pos else xy(z.GetEnd())
        da=(pa[0]-pos[0],pa[1]-pos[1]);dz=(pz[0]-pos[0],pz[1]-pos[1])
        if abs(da[0]*dz[1]-da[1]*dz[0])>1e-5 or da[0]*dz[0]+da[1]*dz[1]>=0:continue
        a.SetStart(vec(pa));a.SetEnd(vec(pz));b.Remove(z);retired.append(z)
        merged+=1;found=True;break
    if not found:break

for z in b.Zones():
    if z.GetZoneName()!='RP2354 LX L2 copper keepout':continue
    for t in b.GetTracks():
        if isinstance(t,k.PCB_VIA) or t.GetNetname() not in NAMES or t.GetLayer()!=k.In2_Cu:continue
        assert not z.Outline().Collide(t.GetEffectiveShape(k.In2_Cu),k.FromMM(.2)),('LED under LX cutout',t.GetNetname(),xy(t.GetStart()),xy(t.GetEnd()))
out=WORK/'final-candidate.kicad_pcb'
assert k.SaveBoard(str(out),b)
shutil.copy2(WORK/'before.kicad_pro',out.with_suffix('.kicad_pro'))
render(b,WORK/'final-routing.png',bounds=(65,53,180,94),scale=20)
print('Removed 2 redundant vias; merged',merged,'collinear segments',flush=True)
