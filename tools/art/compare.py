"""Blueprint vs 3D comparison sheets.

For each class: left = mockup blueprint (idle crop from godot/art/sprites,
also copied to tools/art/blueprints/<class>.png), right = 3D render
(portrait_<class>.png from the model gallery). Writes compare_<class>.png and
compare_all.png into the gallery folder.

Usage: python tools/art/compare.py <gallery_dir>
"""
import pathlib
import shutil
import sys

from PIL import Image, ImageDraw

ROOT = pathlib.Path(__file__).resolve().parents[2]
SPRITES = ROOT / "godot" / "art" / "sprites"
BLUEPRINTS = pathlib.Path(__file__).with_name("blueprints")
CLASSES = ["barbarian", "crusader", "demon_hunter", "monk", "necromancer", "arcanist", "blood_knight", "mage", "archer"]
H = 520
BG = (18, 53, 99, 255)


def fit(img: Image.Image, height: int) -> Image.Image:
    scale = height / img.height
    return img.resize((max(1, int(img.width * scale)), height), Image.LANCZOS)


def trim_render(img: Image.Image) -> Image.Image:
    """Crops the render to the character (non-background pixels)."""
    rgb = img.convert("RGB")
    px = rgb.load()
    w, h = rgb.size
    bg = px[2, 2]
    xs, ys = [], []
    for y in range(0, h, 2):
        for x in range(0, w, 2):
            r, g, b = px[x, y]
            if abs(r - bg[0]) + abs(g - bg[1]) + abs(b - bg[2]) > 40 and not (g > r + 20 and g > b + 20):
                xs.append(x)
                ys.append(y)
    if not xs:
        return img
    pad = 12
    return img.crop((max(0, min(xs) - pad), max(0, min(ys) - pad), min(w, max(xs) + pad), min(h, max(ys) + pad)))


def main() -> None:
    gallery = pathlib.Path(sys.argv[1])
    BLUEPRINTS.mkdir(exist_ok=True)
    rows = []
    for class_id in CLASSES:
        blueprint_src = SPRITES / class_id / "idle.png"
        shutil.copyfile(blueprint_src, BLUEPRINTS / f"{class_id}.png")
        left = fit(Image.open(blueprint_src).convert("RGBA"), H)
        render_path = gallery / f"portrait_{class_id}.png"
        right = fit(trim_render(Image.open(render_path).convert("RGBA")), H)
        sheet = Image.new("RGBA", (left.width + right.width + 60, H + 40), BG)
        sheet.alpha_composite(left, (20, 30))
        sheet.alpha_composite(right, (40 + left.width, 30))
        draw = ImageDraw.Draw(sheet)
        draw.text((20, 8), f"{class_id}: mockup (blueprint)", fill=(255, 220, 120, 255))
        draw.text((40 + left.width, 8), "3D", fill=(255, 220, 120, 255))
        sheet.save(gallery / f"compare_{class_id}.png")
        rows.append(sheet)
    width = max(r.width for r in rows[:3]) * 3
    grid = Image.new("RGBA", (width, (H + 40) * 3), BG)
    for i, row in enumerate(rows):
        thumb = row
        grid.alpha_composite(thumb, ((i % 3) * (width // 3), (i // 3) * (H + 40)))
    grid.thumbnail((2400, 2400))
    grid.save(gallery / "compare_all.png")
    print("compare sheets written to", gallery)


if __name__ == "__main__":
    main()
