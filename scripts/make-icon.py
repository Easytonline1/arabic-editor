#!/usr/bin/env python3
"""يرسم أيقونة «قلم»: ورقة مطوية الزاوية عليها أسطر، وقلم رصاص مائل فوقها.
يكتب icon.ico بالأحجام 16 و24 و32 و48 (BMP بعمق 32 بت) و256 (PNG)،
ويكتب صورة معاينة. الاستعمال: make-icon.py <icon.ico> <preview.png>"""
import io, math, struct, sys
from PIL import Image, ImageDraw

OUTLINE = (52, 64, 84, 255)
PAGE = (255, 255, 255, 255)
FOLD = (214, 222, 233, 255)
LINE = (66, 133, 244, 255)
BODY = (247, 183, 49, 255)
BODY_DARK = (222, 150, 22, 255)
WOOD = (243, 214, 160, 255)
LEAD = (45, 52, 54, 255)
FERRULE = (170, 178, 189, 255)
ERASER = (232, 120, 136, 255)

def draw(size):
    k = 8                                  # رسم بدقة أعلى ثم تصغير لنعومة الحواف
    S = size * k
    f = S / 256.0
    P = lambda pts: [(x * f, y * f) for x, y in pts]
    im = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    ow = max(1, round((10 if size <= 24 else 8) * f))

    # الورقة
    page = [(40, 20), (168, 20), (212, 64), (212, 240), (40, 240)]
    d.polygon(P(page), fill=PAGE, outline=OUTLINE, width=ow)
    d.polygon(P([(168, 20), (168, 64), (212, 64)]), fill=FOLD, outline=OUTLINE, width=ow)

    # الأسطر: أقل في الأحجام الصغيرة حتى تبقى واضحة
    ys = [100, 150] if size <= 24 else [96, 128, 160, 192]
    lw = round((16 if size <= 24 else 10) * f)
    for i, y in enumerate(ys):
        x2 = 176 if i < len(ys) - 1 else 136
        d.line(P([(68, y), (x2, y)]), fill=LINE, width=lw)

    # القلم: محور من الرأس إلى الممحاة باتجاه أعلى اليمين
    tip = (92, 212)
    ang = math.radians(-45)
    ux, uy = math.cos(ang), math.sin(ang)          # اتجاه المحور
    vx, vy = -uy, ux                               # العمودي عليه
    w = 22 if size <= 24 else 19                   # نصف العرض
    def seg(u0, u1, a0, a1):
        pts = [(tip[0] + ux * u0 + vx * a0, tip[1] + uy * u0 + vy * a0),
               (tip[0] + ux * u1 + vx * a0, tip[1] + uy * u1 + vy * a0),
               (tip[0] + ux * u1 + vx * a1, tip[1] + uy * u1 + vy * a1),
               (tip[0] + ux * u0 + vx * a1, tip[1] + uy * u0 + vy * a1)]
        return P(pts)
    pt = lambda u, a: (tip[0] + ux * u + vx * a, tip[1] + uy * u + vy * a)
    L = 190
    # الجسم بلونين ليبدو مضلعًا
    d.polygon(seg(42, 150, -w, 0), fill=BODY)
    d.polygon(seg(42, 150, 0, w), fill=BODY_DARK)
    d.polygon(seg(150, 166, -w, w), fill=FERRULE)
    d.polygon(seg(166, L, -w, w), fill=ERASER)
    d.polygon(P([pt(0, 0), pt(42, -w), pt(42, w)]), fill=WOOD)
    d.polygon(P([pt(0, 0), pt(14, -w / 3), pt(14, w / 3)]), fill=LEAD)
    outline = [pt(0, 0), pt(42, -w), pt(L, -w), pt(L, w), pt(42, w)]
    d.polygon(P(outline), outline=OUTLINE, width=ow)
    if size > 24:
        for u in (42, 150, 166):
            d.line(P([pt(u, -w), pt(u, w)]), fill=OUTLINE, width=max(1, ow // 2))

    return im.resize((size, size), Image.LANCZOS)

def bmp_entry(im):
    """صورة ICO بصيغة DIB: ترويسة BITMAPINFOHEADER بارتفاع مضاعف، وبكسلات BGRA من الأسفل، ثم قناع AND"""
    w, h = im.size
    hdr = struct.pack("<IiiHHIIiiII", 40, w, h * 2, 1, 32, 0, 0, 0, 0, 0, 0)
    px = bytearray()
    for y in range(h - 1, -1, -1):
        for x in range(w):
            r, g, b, a = im.getpixel((x, y))
            px += bytes((b, g, r, a))
    stride = ((w + 31) // 32) * 4
    mask = bytearray()
    for y in range(h - 1, -1, -1):
        row = bytearray(stride)
        for x in range(w):
            if im.getpixel((x, y))[3] == 0:
                row[x // 8] |= 0x80 >> (x % 8)
        mask += row
    return hdr + px + mask

def main(ico_path, preview_path):
    sizes = [16, 24, 32, 48, 256]
    imgs = {s: draw(s) for s in sizes}
    blobs = []
    for s in sizes:
        if s == 256:
            b = io.BytesIO(); imgs[s].save(b, "PNG"); blobs.append(b.getvalue())
        else:
            blobs.append(bmp_entry(imgs[s]))
    out = struct.pack("<HHH", 0, 1, len(sizes))
    off = 6 + 16 * len(sizes)
    for s, b in zip(sizes, blobs):
        out += struct.pack("<BBBBHHII", s % 256, s % 256, 0, 0, 1, 32, len(b), off)
        off += len(b)
    with open(ico_path, "wb") as fh:
        fh.write(out + b"".join(blobs))

    # معاينة: كل حجم بحجمه الحقيقي ثم مكبّرًا بلا تنعيم
    W = 256 + 48 + 5 * 136
    pv = Image.new("RGBA", (W, 300), (255, 255, 255, 255))
    pv.alpha_composite(imgs[256], (16, 22))
    x = 300
    for s in [16, 24, 32, 48]:
        big = imgs[s].resize((s * 4 if s <= 32 else 128, s * 4 if s <= 32 else 128), Image.NEAREST)
        pv.alpha_composite(big, (x, 40))
        pv.alpha_composite(imgs[s], (x, 220))
        x += 150
    pv.save(preview_path)

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
