"""Extracts character/enemy sprites from the class mockups in arts/.

For every box in sprite_boxes.json: crop -> ML background removal (rembg)
-> keep the largest opaque blob -> trim -> save as
godot/art/sprites/<id>/<pose>.png. Also writes a contact sheet for review.

Usage: python tools/art/extract_sprites.py [--only id1,id2] [--sheet out.png]
Requires: pip install rembg onnxruntime pillow
"""
import argparse
import json
import pathlib

from PIL import Image, ImageDraw
from rembg import new_session, remove

ROOT = pathlib.Path(__file__).resolve().parents[2]
ARTS = ROOT / "arts"
OUT = ROOT / "godot" / "art" / "sprites"
BOXES = pathlib.Path(__file__).with_name("sprite_boxes.json")
ALPHA_THRESHOLD = 24


def largest_blob(img: Image.Image) -> Image.Image:
    """Drops stray fragments (neighbouring enemies, effects) keeping the main subject."""
    alpha = img.getchannel("A")
    w, h = img.size
    px = alpha.load()
    seen = bytearray(w * h)
    best = []
    for y in range(h):
        for x in range(w):
            i = y * w + x
            if seen[i] or px[x, y] < ALPHA_THRESHOLD:
                continue
            stack = [(x, y)]
            seen[i] = 1
            blob = []
            while stack:
                cx, cy = stack.pop()
                blob.append((cx, cy))
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if 0 <= nx < w and 0 <= ny < h:
                        j = ny * w + nx
                        if not seen[j] and px[nx, ny] >= ALPHA_THRESHOLD:
                            seen[j] = 1
                            stack.append((nx, ny))
            if len(blob) > len(best):
                best = blob
    mask = Image.new("L", (w, h), 0)
    mpx = mask.load()
    for x, y in best:
        mpx[x, y] = 255
    # Keep soft edges: original alpha inside the dilated blob mask.
    from PIL import ImageFilter
    mask = mask.filter(ImageFilter.MaxFilter(5))
    out = img.copy()
    out.putalpha(Image.composite(alpha, Image.new("L", (w, h), 0), mask))
    return out


def apply_erase(cut: Image.Image, box, scale: int, shapes) -> Image.Image:
    """Clears mockup leftovers (captions, range bars, floor discs) given in source coords."""
    alpha = cut.getchannel("A")
    draw = ImageDraw.Draw(alpha)
    to_local = lambda x, y: ((x - box[0]) * scale, (y - box[1]) * scale)
    for shape in shapes:
        if "rect" in shape:
            x0, y0, x1, y1 = shape["rect"]
            draw.rectangle([*to_local(x0, y0), *to_local(x1, y1)], fill=0)
        elif "ellipse" in shape:
            cx, cy, rx, ry = shape["ellipse"]
            keep = alpha.copy()
            draw.ellipse([*to_local(cx - rx, cy - ry), *to_local(cx + rx, cy + ry)], fill=0)
            if "protect" in shape:
                px0, py0, px1, py1 = shape["protect"]
                region = [int(v) for v in (*to_local(px0, py0), *to_local(px1, py1))]
                alpha.paste(keep.crop(region), region[:2])
    keys = [shape for shape in shapes if "key" in shape]
    if keys:
        rgb = cut.convert("RGB").load()
        apx = alpha.load()
        for shape in keys:
            kr, kg, kb = shape["key"]
            tol2 = shape.get("tol", 50) ** 2
            y0 = max(0, int(to_local(0, shape["band"][0])[1]))
            y1 = min(cut.height, int(to_local(0, shape["band"][1])[1]))
            for y in range(y0, y1):
                for x in range(cut.width):
                    r, g, b = rgb[x, y]
                    if (r - kr) ** 2 + (g - kg) ** 2 + (b - kb) ** 2 <= tol2:
                        apx[x, y] = 0
    out = cut.copy()
    out.putalpha(alpha)
    return out


def process(session, source: Image.Image, spec, matting: bool = True) -> Image.Image:
    box = spec["box"] if isinstance(spec, dict) else spec
    crop = source.crop(tuple(box)).convert("RGBA")
    # Upscale small crops so the matting model sees enough detail.
    scale = max(1, round(256 / min(crop.size)))
    if scale > 1:
        crop = crop.resize((crop.width * scale, crop.height * scale), Image.LANCZOS)
    cut = remove(crop, session=session, alpha_matting=matting)
    if isinstance(spec, dict) and spec.get("erase"):
        cut = apply_erase(cut, box, scale, spec["erase"])
    cut = largest_blob(cut)
    bbox = cut.getchannel("A").point(lambda a: 255 if a >= ALPHA_THRESHOLD else 0).getbbox()
    return cut.crop(bbox) if bbox else cut


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--only", default="")
    parser.add_argument("--sheet", default="")
    parser.add_argument("--model", default="isnet-general-use")
    parser.add_argument("--no-matting", action="store_true")
    args = parser.parse_args()
    boxes = json.loads(BOXES.read_text(encoding="utf-8"))
    only = {s for s in args.only.split(",") if s}
    session = new_session(args.model)
    results = []
    for sprite_id, spec in boxes.items():
        if sprite_id.startswith("_") or (only and sprite_id not in only):
            continue
        source = Image.open(ARTS / spec["source"]).convert("RGB")
        for pose, box in spec.items():
            if pose == "source":
                continue
            image = process(session, source, box, not args.no_matting)
            target = OUT / sprite_id / f"{pose}.png"
            target.parent.mkdir(parents=True, exist_ok=True)
            image.save(target)
            results.append((f"{sprite_id}/{pose}", image))
            print("saved", target.relative_to(ROOT), image.size)
    if args.sheet and results:
        cell = 170
        cols = 6
        rows = (len(results) + cols - 1) // cols
        sheet = Image.new("RGBA", (cols * cell, rows * (cell + 16)), (60, 120, 60, 255))
        draw = ImageDraw.Draw(sheet)
        for i, (label, image) in enumerate(results):
            thumb = image.copy()
            thumb.thumbnail((cell - 8, cell - 8))
            x = (i % cols) * cell
            y = (i // cols) * (cell + 16)
            sheet.alpha_composite(thumb, (x + (cell - thumb.width) // 2, y + 16 + (cell - thumb.height) // 2))
            draw.text((x + 4, y + 2), label, fill=(255, 255, 255, 255))
        sheet.save(args.sheet)
        print("sheet", args.sheet)


if __name__ == "__main__":
    main()
