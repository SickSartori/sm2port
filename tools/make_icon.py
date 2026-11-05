# Makes res/sm2.ico and docs/logo.png:
#   python -I tools/make_icon.py <font.ttf> <output folder>
# The font is Impact (C:\Windows\Fonts\impact.ttf on Windows); any heavy sans works. Only the rendered pictures
# are kept in the repository, not the font.
"""The sm2port icon: "SM" in chrome and a big glossy red "2", both slanted, outlined and extruded in 3D, on a dark blue
tile with a faint spider web, a blue glow and a skewed red bar. Original artwork, not the game's or Marvel's logo."""
import sys
from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageChops

FONT = sys.argv[1]
OUT = sys.argv[2]  # folder for icon-256.png (docs/logo.png), icon-sheet.png and sm2.ico (res/sm2.ico)
S = 1024           # master size; every icon size is drawn from it

CHROME = [(0, (255, 255, 255, 255)), (0.45, (205, 208, 215, 255)), (0.5, (110, 112, 120, 255)),
          (0.62, (150, 152, 160, 255)), (1, (245, 246, 250, 255))]
RED = [(0, (255, 120, 125, 255)), (0.42, (235, 40, 52, 255)), (0.5, (160, 8, 18, 255)), (1, (205, 20, 32, 255))]
BAND = [(0, (230, 50, 60, 255)), (0.76, (190, 16, 26, 255)), (0.77, (150, 8, 16, 255)), (1, (120, 4, 10, 255))]
EXTRUSION = [(0, (70, 10, 16, 255)), (1, (18, 4, 6, 255))]


def gradient(size, stops):
    """A vertical gradient through (position, colour) stops."""
    w, h = size
    g = Image.new("RGBA", size)
    px = g.load()
    for y in range(h):
        t = y / (h - 1)
        c = stops[-1][1]
        for (p0, c0), (p1, c1) in zip(stops, stops[1:]):
            if p0 <= t <= p1:
                u = (t - p0) / (p1 - p0) if p1 > p0 else 0
                c = tuple(round(c0[k] + (c1[k] - c0[k]) * u) for k in range(4))
                break
        for x in range(w):
            px[x, y] = c
    return g


def rounded_mask(size, radius):
    m = Image.new("L", (size, size), 0)
    ImageDraw.Draw(m).rounded_rectangle((0, 0, size - 1, size - 1), radius=radius, fill=255)
    return m


def grow(mask, pixels):
    return mask.filter(ImageFilter.MaxFilter(pixels * 2 + 1)) if pixels > 0 else mask


def text_mask(text, size=600, slant=0.25):
    """The text as a mask, sheared to the right like the game's logo, cropped tight."""
    font = ImageFont.truetype(FONT, size)
    box = font.getbbox(text)
    tmp = Image.new("L", (box[2] - box[0] + size * 2, box[3] - box[1] + size), 0)
    ImageDraw.Draw(tmp).text((size - box[0], size // 2 - box[1]), text, font=font, fill=255)
    tmp = tmp.transform(tmp.size, Image.AFFINE, (1, slant, -slant * tmp.height / 2, 0, 1, 0), Image.BICUBIC)
    return tmp.crop(tmp.getbbox())


def place(mask, height_ratio, centre, max_width):
    """The mask scaled to `height_ratio` of the tile (never wider than `max_width`), centred at `centre`."""
    scale = S * height_ratio / mask.height
    if mask.width * scale > S * max_width:
        scale = S * max_width / mask.width
    m = mask.resize((round(mask.width * scale), round(mask.height * scale)), Image.LANCZOS)
    out = Image.new("L", (S, S), 0)
    out.paste(m, (round(S * centre[0] - m.width / 2), round(S * centre[1] - m.height / 2)))
    return out


def tile():
    """Rounded dark square: lighter at the top, faint diagonal streaks, a red glow low right and the red bar."""
    img = gradient((S, S), [(0, (30, 40, 86, 255)), (1, (6, 8, 20, 255))])
    glow = Image.new("L", (S, S), 0)
    ImageDraw.Draw(glow).ellipse((S * 0.25, S * 0.45, S * 1.30, S * 1.40), fill=175)
    glow = glow.filter(ImageFilter.GaussianBlur(S * 0.13))
    img = Image.composite(Image.new("RGBA", (S, S), (30, 80, 210, 255)), img, glow)
    web = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(web)
    ox, oy = S * 0.05, S * 0.05  # the web hangs from the top left corner
    for i in range(0, 8):  # spokes
        a = i * (3.1416 / 2) / 7
        d.line([(ox, oy), (ox + S * 1.6 * __import__("math").cos(a), oy + S * 1.6 * __import__("math").sin(a))],
               fill=(255, 255, 255, 26), width=round(S * 0.006))
    for r in (0.22, 0.4, 0.6, 0.82, 1.06):  # sagging threads between the spokes
        pts = []
        for i in range(0, 8):
            a = i * (3.1416 / 2) / 7
            pts.append((ox + S * r * __import__("math").cos(a), oy + S * r * __import__("math").sin(a)))
        for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
            mx, my = (x0 + x1) / 2 - S * 0.02 * r, (y0 + y1) / 2 - S * 0.02 * r
            d.line([(x0, y0), (mx, my), (x1, y1)], fill=(255, 255, 255, 26), width=round(S * 0.006))
    img = Image.alpha_composite(img, web)
    y0, y1, skew = S * 0.70, S * 0.84, S * 0.07
    bar = Image.new("L", (S, S), 0)
    ImageDraw.Draw(bar).polygon([(-S * 0.2 + skew, y0), (S * 1.2 + skew, y0), (S * 1.2, y1), (-S * 0.2, y1)], fill=255)
    img = Image.composite(gradient((S, S), BAND), img, bar)
    out = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    out.paste(img, (0, 0), rounded_mask(S, S * 0.2))
    rim = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    ImageDraw.Draw(rim).rounded_rectangle((S * 0.006, S * 0.006, S * 0.994, S * 0.994), radius=S * 0.2,
                                          outline=(255, 255, 255, 40), width=round(S * 0.008))
    return Image.alpha_composite(out, rim)


def emboss(img, mask, fill, outline, depth, chrome_edge):
    """Letters as: soft drop shadow, 3D extrusion down-right, dark outline, optional chrome edge, glossy fill."""
    outer = grow(mask, outline)
    extrusion = Image.new("L", (S, S), 0)
    for i in range(depth, 0, -1):
        extrusion = ImageChops.lighter(extrusion, ImageChops.offset(outer, round(i * 1.0), round(i * 1.6)))
    shadow = extrusion.filter(ImageFilter.GaussianBlur(S * 0.012))
    img = Image.composite(Image.new("RGBA", (S, S), (0, 0, 0, 200)), img,
                          ImageChops.offset(shadow, round(S * 0.012), round(S * 0.02)))
    img = Image.composite(gradient((S, S), EXTRUSION), img, extrusion)
    img = Image.composite(Image.new("RGBA", (S, S), (16, 8, 10, 255)), img, outer)
    if chrome_edge:
        img = Image.composite(gradient((S, S), CHROME), img, grow(mask, max(2, outline // 3)))
    face = gradient((S, S), fill)
    gloss = Image.new("L", (S, S), 0)
    ImageDraw.Draw(gloss).polygon([(0, 0), (S, 0), (S, S * 0.40), (0, S * 0.47)], fill=55)
    face = Image.composite(Image.new("RGBA", (S, S), (255, 255, 255, 255)), face, gloss)
    return Image.composite(face, img, mask)


def master(outline, depth):
    img = tile()
    three = place(text_mask("2"), 0.62, (0.66, 0.50), 0.50)
    mc = place(text_mask("SM"), 0.32, (0.35, 0.30), 0.60)
    img = emboss(img, three, RED, outline + 4, depth, chrome_edge=True)
    return emboss(img, mc, CHROME, outline, round(depth * 0.65), chrome_edge=False)


big = master(outline=10, depth=22)    # 48 px and up
small = master(outline=22, depth=30)  # 16-32 px: thicker outlines, or they vanish
sizes = [256, 128, 64, 48, 40, 32, 24, 20, 16]
frames = [(big if n >= 48 else small).resize((n, n), Image.LANCZOS) for n in sizes]
big.resize((256, 256), Image.LANCZOS).save(OUT + "/icon-256.png")
sheet = Image.new("RGBA", (256 + 16 + sum(n + 16 for n in sizes[1:]), 256), (40, 40, 44, 255))
x = 0
for f in frames:
    sheet.paste(f, (x, 256 - f.height), f)
    x += f.width + 16
sheet.save(OUT + "/icon-sheet.png")
frames[0].save(OUT + "/sm2.ico", format="ICO", sizes=[(n, n) for n in sizes], append_images=frames[1:])
print("ok")
