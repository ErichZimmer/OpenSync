"""Inspect native KiCad filled copper, without changing the board."""
import sys
from pathlib import Path
import pcbnew as k
from PIL import Image,ImageDraw
from pcb_helpers import xy,_polygons
ROOT=Path(__file__).resolve().parents[2]
def render(b,path,bounds,scale=55,layer=k.F_Cu):
    x0,y0,x1,y1=bounds
    im=Image.new('RGB',(round((x1-x0)*scale),round((y1-y0)*scale)),(9,17,24));d=ImageDraw.Draw(im)
    def pt(p):return ((p[0]-x0)*scale,(p[1]-y0)*scale)
    colors={'GND':(43,89,83),'+3V3':(100,48,51),'+5V':(109,91,42),'+1V1':(70,66,112)}
    for z in b.Zones():
        if z.GetIsRuleArea() or not z.IsOnLayer(layer) or not z.HasFilledPolysForLayer(layer):continue
        poly=z.GetFilledPolysList(layer);mask=Image.new('1',im.size,0);md=ImageDraw.Draw(mask)
        for n in range(poly.OutlineCount()):
            o=poly.COutline(n);md.polygon([pt(xy(o.CPoint(i))) for i in range(o.PointCount())],fill=1)
            for h in range(poly.HoleCount(n)):
                o=poly.CHole(n,h);md.polygon([pt(xy(o.CPoint(i))) for i in range(o.PointCount())],fill=0)
        im.paste(colors.get(z.GetNetname(),(65,73,83)),mask=mask)
    d=ImageDraw.Draw(im)
    for t in b.GetTracks():
        if not t.IsOnLayer(layer) or isinstance(t,k.PCB_VIA):continue
        d.line([pt(xy(t.GetStart())),pt(xy(t.GetEnd()))],fill=colors.get(t.GetNetname(),(194,111,61)),width=max(1,round(k.ToMM(t.GetWidth())*scale)))
    for f in b.GetFootprints():
        for p in f.Pads():
            if not p.IsOnLayer(layer):continue
            poly=k.SHAPE_POLY_SET();p.TransformShapeToPolygon(poly,layer,0,k.FromMM(.005),k.ERROR_OUTSIDE)
            for shape in _polygons(poly):
                if len(shape)>2:d.polygon([pt(x) for x in shape],fill=(191,147,86),outline=(255,216,144))
            x,y=pt(xy(p));d.text((x-3,y-4),p.GetNumber(),fill=(15,17,17))
        x,y=xy(f)
        if x0<x<x1 and y0<y<y1:d.text(pt((x,y-1.1)),f.GetReference(),fill=(232,239,228))
    for v in b.GetTracks():
        if not isinstance(v,k.PCB_VIA):continue
        x,y=pt(xy(v));r=k.ToMM(v.GetWidth(layer))*scale/2;dr=k.ToMM(v.GetDrill())*scale/2
        d.ellipse((x-r,y-r,x+r,y+r),fill=(170,157,105));d.ellipse((x-dr,y-dr,x+dr,y+dr),fill=(9,17,24))
    im.save(path)
if __name__=='__main__':
    b=k.LoadBoard(sys.argv[1] if len(sys.argv)>1 else str(ROOT/'tmp/routing_work/integrated.kicad_pcb'))
    render(b,ROOT/'tmp/routing_work/power-filled.png',(38,66,52,104),40)
    render(b,ROOT/'tmp/routing_work/mcu-filled.png',(78,75,97,99),50)
