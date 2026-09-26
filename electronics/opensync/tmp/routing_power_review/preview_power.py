"""Read native filled copper and render a project-local inspection image."""
import pcbnew as k
from PIL import Image,ImageDraw,ImageFont
from pathlib import Path
b=k.LoadBoard(str(Path(__file__).with_name('power-test.kicad_pcb')))
views=[('U31 input mux',(40,67,49.5,76)),('U32 5V converter',(39.5,76,51,88.5)),('U37 3V3 converter',(35,90,50,104))]
scale=85
font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',18)
small=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',14)
colors={'GND':'#295f78','+5V':'#aa443a','+3V3':'#78602b','VBUS':'#6f357e','/Power Interface/VOUT':'#237453','/Power Interface/VDC':'#496129'}
def color(net): return colors.get(net,'#7b3980' if '-SW)' in net or '-L' in net else '#808080')
panels=[]
for title,(x0,y0,x1,y1) in views:
    image=Image.new('RGB',(int((x1-x0)*scale),int((y1-y0)*scale)+40),'#101b25')
    draw=ImageDraw.Draw(image)
    def xy(v): return ((k.ToMM(v.x)-x0)*scale,(k.ToMM(v.y)-y0)*scale+40)
    def contour(c): return [xy(c.CPoint(i)) for i in range(c.PointCount())]
    for z in b.Zones():
        if z.GetLayer()!=k.F_Cu or z.GetIsRuleArea(): continue
        polys=z.GetFilledPolysList(k.F_Cu)
        mask=Image.new('L',image.size); md=ImageDraw.Draw(mask)
        for i in range(polys.OutlineCount()):
            pts=contour(polys.COutline(i))
            if len(pts)>2: md.polygon(pts,fill=255)
            for h in range(polys.HoleCount(i)):
                pts=contour(polys.CHole(i,h))
                if len(pts)>2: md.polygon(pts,fill=0)
        image.paste(color(z.GetNetname()),mask=mask)
    draw=ImageDraw.Draw(image)
    for t in b.GetTracks():
        if isinstance(t,k.PCB_VIA):
            x,y=xy(t.GetPosition()); r=k.ToMM(t.GetWidth(k.F_Cu))*scale/2; h=k.ToMM(t.GetDrill())*scale/2
            draw.ellipse((x-r,y-r,x+r,y+r),fill=color(t.GetNetname()),outline='#dfdfdf')
            draw.ellipse((x-h,y-h,x+h,y+h),fill='#101b25')
        elif t.GetLayer()==k.F_Cu:
            a,c=xy(t.GetStart()),xy(t.GetEnd()); w=max(1,round(k.ToMM(t.GetWidth())*scale))
            draw.line((a,c),fill=color(t.GetNetname()),width=w)
            for x,y in [a,c]: draw.ellipse((x-w/2,y-w/2,x+w/2,y+w/2),fill=color(t.GetNetname()))
    for f in b.GetFootprints():
        fp=f.GetPosition(); fx,fy=k.ToMM(fp.x),k.ToMM(fp.y)
        if not (x0<fx<x1 and y0<fy<y1): continue
        for p in f.Pads():
            if not p.IsOnLayer(k.F_Cu): continue
            polys=p.GetEffectivePolygon(k.F_Cu)
            for i in range(polys.OutlineCount()):
                points=contour(polys.COutline(i))
                if len(points)>2: draw.polygon(points,fill=color(p.GetNetname()),outline='#eeeeee',width=2)
            px,py=xy(p.GetPosition()); draw.text((px,py),p.GetNumber(),font=small,fill='white',anchor='mm')
        px,py=xy(fp); draw.text((px,py),f.GetReference(),font=small,fill='#ffff77',anchor='mm')
    draw.rectangle((0,0,image.width,38),fill='#101b25'); draw.text((12,8),title,font=font,fill='white')
    panels.append(image)
result=Image.new('RGB',(sum(p.width for p in panels)+40,max(p.height for p in panels)),'#101b25')
offset=0
for p in panels: result.paste(p,(offset,0)); offset+=p.width+20
result.save(str(Path(__file__).with_name('power-preview.png')))
