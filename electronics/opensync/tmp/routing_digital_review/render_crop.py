import pcbnew as p
from PIL import Image, ImageDraw, ImageFont
import sys

boardfile, out = sys.argv[1:3]
crop = tuple(map(float, sys.argv[3:7])) if len(sys.argv)>3 else (60,60,95,96)
scale=55
x0,y0,x1,y1=crop
im=Image.new('RGB',(int((x1-x0)*scale),int((y1-y0)*scale)),(20,25,30))
d=ImageDraw.Draw(im)
b=p.LoadBoard(boardfile)
def xy(v): return ((p.ToMM(v.x)-x0)*scale,(p.ToMM(v.y)-y0)*scale)
def tup(x,y): return ((x-x0)*scale,(y-y0)*scale)
for n in range(int(x0),int(x1)+1):
    d.line([tup(n,y0),tup(n,y1)],fill=(38,43,48))
    d.text(tup(n,y0),str(n),fill='gray')
for n in range(int(y0),int(y1)+1):
    d.line([tup(x0,n),tup(x1,n)],fill=(38,43,48))
    d.text(tup(x0,n),str(n),fill='gray')
for layer in (p.In2_Cu,p.F_Cu):
    for tr in b.GetTracks():
        if isinstance(tr,p.PCB_VIA) or tr.GetLayer()!=layer: continue
        d.line([xy(tr.GetStart()),xy(tr.GetEnd())],fill=(240,65,55) if layer==p.F_Cu else (40,130,45),width=max(1,round(p.ToMM(tr.GetWidth())*scale)))
for f in b.GetFootprints():
    for pad in f.Pads():
        if not pad.IsOnLayer(p.F_Cu):continue
        bb=pad.GetBoundingBox(); v0=xy(bb.GetPosition()); v1=(v0[0]+p.ToMM(bb.GetWidth())*scale,v0[1]+p.ToMM(bb.GetHeight())*scale)
        d.rectangle([v0,v1], fill=(130,40,35),outline=(245,100,70))
        if x0<=p.ToMM(pad.GetPosition().x)<=x1 and y0<=p.ToMM(pad.GetPosition().y)<=y1:
            d.text((v0[0]+2,v0[1]+2),pad.GetNumber(),fill='white')
    pos=xy(f.GetPosition())
    if -50<pos[0]<im.width+50 and -50<pos[1]<im.height+50:
        d.text((pos[0]+5,pos[1]-18),f.GetReference(),fill=(255,255,110))
for tr in b.GetTracks():
    if not isinstance(tr,p.PCB_VIA):continue
    cx,cy=xy(tr.GetPosition()); r=p.ToMM(tr.GetWidth(p.F_Cu))*scale/2
    d.ellipse((cx-r,cy-r,cx+r,cy+r),outline=(210,210,210),fill=(60,75,90))
    if x0<=p.ToMM(tr.GetPosition().x)<=x1 and y0<=p.ToMM(tr.GetPosition().y)<=y1:
        net=tr.GetNetname().split('/')[-1]
        if net in ('GND','+3V3','+1V1'):d.text((cx+4,cy-5),net,fill='cyan')
im.save(out)
