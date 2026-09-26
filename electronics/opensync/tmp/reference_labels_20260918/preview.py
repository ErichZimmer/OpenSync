"""Render native KiCad reference glyph geometry and pads for verification."""
from pathlib import Path
import pcbnew as k
from PIL import Image, ImageDraw

work = Path(__file__).resolve().parent
board = k.LoadBoard(str(work / 'candidate.kicad_pcb'))
scale = 13
origin = (70, 65)
image = Image.new('RGB', (1521, 1014), (15, 27, 31))
draw = ImageDraw.Draw(image)


def xy(v):
    return ((k.ToMM(v.x) - origin[0]) * scale, (k.ToMM(v.y) - origin[1]) * scale)


def polygon(poly, color):
    for i in range(poly.OutlineCount()):
        outline = poly.COutline(i)
        points = [xy(outline.CPoint(j)) for j in range(outline.PointCount())]
        if len(points) >= 3:
            draw.polygon(points, fill=color)
        for h in range(poly.HoleCount(i)):
            hole = poly.CHole(i, h)
            draw.polygon([xy(hole.CPoint(j)) for j in range(hole.PointCount())], fill=(15, 27, 31))


for footprint in board.GetFootprints():
    if not footprint.GetSheetname().startswith('/Output Channel '):
        continue
    for pad in footprint.Pads():
        if not pad.IsOnLayer(k.F_Cu):
            continue
        poly = k.SHAPE_POLY_SET()
        pad.TransformShapeToPolygon(poly, k.F_Cu, 0, k.FromMM(.01), k.ERROR_OUTSIDE)
        polygon(poly, (132, 58, 52))
    field = footprint.Reference()
    if field.IsVisible():
        poly = k.SHAPE_POLY_SET()
        field.TransformTextToPolySet(poly, 0, k.FromMM(.005), k.ERROR_INSIDE)
        polygon(poly, (244, 238, 137))

image.save(work / 'reference-preview.png')
print(work / 'reference-preview.png')
