# Makes installer/wizard-*.png and installer/wizard-small-*.png (the installer's side picture and small icon, at 100,
# 150, 200 and 250 % scale) from the launcher's pictures and the program icon:
#   python -I tools/make_installer_images.py
"""The sm2port installer pictures: a night skyline (one of the launcher's generated backgrounds) under a dark gradient,
the name in Barlow Black Italic, the red bar of the logo, and the program icon."""
import pathlib
from PIL import Image, ImageDraw, ImageFont

ROOT = pathlib.Path(__file__).resolve().parent.parent
FONT = str(ROOT / "third_party/fonts/BarlowSemiCondensed-BlackItalic.ttf")
FONT_SMALL = str(ROOT / "third_party/fonts/BarlowSemiCondensed-SemiBold.ttf")
SCALES = {100: (164, 314, 55), 150: (246, 471, 83), 200: (328, 628, 110), 250: (410, 785, 138)}
RED = (222, 36, 48)


def cover(img, w, h):
    s = max(w / img.width, h / img.height)
    img = img.resize((round(img.width * s), round(img.height * s)), Image.LANCZOS)
    return img.crop(((img.width - w) // 2, (img.height - h) // 2, (img.width - w) // 2 + w, (img.height - h) // 2 + h))


def side(w, h):
    back = Image.open(ROOT / "res/launcher_bg_4.jpg").convert("RGB")
    img = cover(back, w, h)
    shade = Image.new("RGBA", (w, h))
    px = shade.load()
    for y in range(h):
        t = y / (h - 1)
        a = round(255 * (0.55 + 0.4 * t))  # darker down the picture
        for x in range(w):
            px[x, y] = (8, 8, 14, a)
    img = Image.alpha_composite(img.convert("RGBA"), shade)
    d = ImageDraw.Draw(img)
    u = w / 164
    d.text((14 * u, 26 * u), "SM2", font=ImageFont.truetype(FONT, round(46 * u)), fill=(240, 241, 246))
    d.text((14 * u, 72 * u), "PORT", font=ImageFont.truetype(FONT, round(46 * u)), fill=(240, 241, 246))
    d.text((15 * u, 126 * u), "SPIDER-MAN 2 · PC", font=ImageFont.truetype(FONT_SMALL, round(13 * u)), fill=RED)
    for y0, y1, alpha in ((150, 156, 255), (160, 163, 150)):  # the logo's skewed red bars
        d.polygon([(10 * u + 6 * u, y0 * u), (w - 14 * u + 6 * u, y0 * u), (w - 14 * u, y1 * u), (10 * u, y1 * u)], fill=RED + (alpha,))
    icon = Image.open(ROOT / "docs/logo.png").convert("RGBA")
    size = round(w * 0.56)
    icon = icon.resize((size, size), Image.LANCZOS)
    img.alpha_composite(icon, ((w - size) // 2, round(h * 0.62)))
    return img.convert("RGB")


def small(n):
    icon = Image.open(ROOT / "docs/logo.png").convert("RGBA").resize((n, n), Image.LANCZOS)
    img = Image.new("RGB", (n, n), (14, 14, 22))
    img.paste(icon, (0, 0), icon)
    return img


out = ROOT / "installer"
out.mkdir(exist_ok=True)
for pct, (w, h, n) in SCALES.items():
    side(w, h).save(out / f"wizard-{pct}.png", optimize=True)
    small(n).save(out / f"wizard-small-{pct}.png", optimize=True)
print("ok")
