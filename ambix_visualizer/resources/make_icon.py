"""
Generates the ambix_visualizer app icon (PNG).

Concept: a dark sphere seen top-down with meridians/parallels and a handful of
coloured source "pucks" sampled from the app's jet colormap, so the icon reads
as "spatial audio positions on a sphere" at a glance.

Usage:
    python3 make_icon.py
Produces: icon.png (1024x1024)
"""

import math
from PIL import Image, ImageDraw, ImageFilter

SIZE = 1024
BG = (14, 18, 28, 255)            # near-black navy, high contrast against sphere
SPHERE_OUTER = (22, 28, 42, 255)  # slightly lighter ring/frame
SPHERE_INNER_HI = (60, 70, 95, 255)
SPHERE_INNER_LO = (16, 18, 28, 255)
GRID = (255, 255, 255, 60)
GRID_BOLD = (255, 255, 255, 95)
EQUATOR = (255, 255, 255, 150)
OUTLINE = (255, 255, 255, 200)


def jet_stop(t: float):
    stops = [
        (0.000, (0, 0, 128)),
        (0.125, (0, 0, 255)),
        (0.375, (0, 255, 255)),
        (0.500, (0, 255, 0)),
        (0.625, (255, 255, 0)),
        (0.875, (255, 0, 0)),
        (1.000, (128, 0, 0)),
    ]
    for i in range(1, len(stops)):
        t0, c0 = stops[i - 1]
        t1, c1 = stops[i]
        if t <= t1:
            u = 0.0 if t1 == t0 else (t - t0) / (t1 - t0)
            return tuple(int(c0[k] + u * (c1[k] - c0[k])) for k in range(3))
    return stops[-1][1]


def main() -> None:
    # High-res canvas, downsampled at the end for crisp edges.
    scale = 2
    W = SIZE * scale
    img = Image.new("RGBA", (W, W), BG)
    draw = ImageDraw.Draw(img, "RGBA")

    cx = cy = W // 2
    # Leave margin for macOS/iOS safe area in icon frames.
    radius = int(W * 0.40)

    # Soft outer halo
    halo = Image.new("RGBA", (W, W), (0, 0, 0, 0))
    hd = ImageDraw.Draw(halo)
    for i in range(40):
        a = int(90 * (1.0 - i / 40))
        r = radius + int(i * W * 0.008)
        hd.ellipse(
            [(cx - r, cy - r), (cx + r, cy + r)],
            outline=(120, 180, 255, a),
            width=2,
        )
    halo = halo.filter(ImageFilter.GaussianBlur(radius=8))
    img = Image.alpha_composite(img, halo)
    draw = ImageDraw.Draw(img, "RGBA")

    # Sphere body with radial gradient (fake it via concentric circles)
    steps = 120
    for i in range(steps, 0, -1):
        t = i / steps  # 1 at outside, 0 at centre
        r = int(radius * t)
        # Mix gradient colours (centre bright → edge dark)
        def mix(a, b, u):
            return tuple(int(a[k] + u * (b[k] - a[k])) for k in range(4))
        col = mix(SPHERE_INNER_HI, SPHERE_INNER_LO, t)
        draw.ellipse(
            [(cx - r, cy - r), (cx + r, cy + r)],
            fill=col,
        )

    # Grid: parallels + meridians, projected top-down
    # Parallels: concentric circles at elevations -60, -30, 0, +30, +60 (0=centre)
    def parallel_r(ele_deg):
        return radius * math.cos(math.radians(ele_deg))

    for ele in (-60, -30, 0, 30, 60):
        r = int(parallel_r(ele))
        col = EQUATOR if ele == 0 else (GRID_BOLD if ele % 60 == 0 else GRID)
        width = 4 if ele == 0 else (3 if ele % 60 == 0 else 2)
        draw.ellipse(
            [(cx - r, cy - r), (cx + r, cy + r)],
            outline=col,
            width=width,
        )

    # Meridians: straight lines through the centre at azimuths 0, 30, 60, ...
    for az in range(0, 360, 30):
        a = math.radians(az)
        x2 = cx + int(radius * math.sin(a))
        y2 = cy - int(radius * math.cos(a))
        col = GRID_BOLD if az % 90 == 0 else GRID
        width = 3 if az % 90 == 0 else 2
        draw.line([(cx, cy), (x2, y2)], fill=col, width=width)

    # Outline
    draw.ellipse(
        [(cx - radius, cy - radius), (cx + radius, cy + radius)],
        outline=OUTLINE,
        width=4,
    )

    # Pucks: positioned in map-style top-down (az cw from top, ele controls
    # radial distance). Colours sampled from the jet colormap to mimic live
    # RMS levels in the app.
    def puck(az_deg, ele_deg, rms_t, size_px):
        az = math.radians(az_deg)
        r_from_centre = radius * math.cos(math.radians(ele_deg))
        x = cx + r_from_centre * math.sin(az)
        y = cy - r_from_centre * math.cos(az)
        rgb = jet_stop(rms_t)
        # Outer peak ring (white-ish)
        ring_r = size_px + 14
        draw.ellipse(
            [(x - ring_r, y - ring_r), (x + ring_r, y + ring_r)],
            outline=(255, 255, 255, 200),
            width=5,
        )
        # Puck body
        draw.ellipse(
            [(x - size_px, y - size_px), (x + size_px, y + size_px)],
            fill=(*rgb, 255),
            outline=(0, 0, 0, 180),
            width=3,
        )

    # Scale sizes with W
    S = W / 1024
    puck(  0,  0, 0.85, int(58 * S))   # hot orange, FRONT
    puck(-90,  30, 0.55, int(50 * S))  # yellow-green, LEFT-upper
    puck( 90,  10, 0.35, int(46 * S))  # cyan, RIGHT
    puck(135, -15, 0.15, int(42 * S))  # deep blue, behind-right-low
    puck(200,  45, 0.70, int(44 * S))  # warm yellow, behind-upper-left

    # Subtle vignette so the content pops toward the centre
    vignette = Image.new("RGBA", (W, W), (0, 0, 0, 0))
    vd = ImageDraw.Draw(vignette)
    vd.ellipse(
        [(0, 0), (W, W)],
        fill=(0, 0, 0, 110),
    )
    inner = Image.new("RGBA", (W, W), (0, 0, 0, 0))
    ind = ImageDraw.Draw(inner)
    ind.ellipse(
        [(cx - radius - 10, cy - radius - 10),
         (cx + radius + 10, cy + radius + 10)],
        fill=(0, 0, 0, 0),
    )
    # Skip complex vignette; keep the icon clean.

    # Downsample for anti-aliased final image
    img = img.resize((SIZE, SIZE), Image.LANCZOS)

    out_path = __file__.replace("make_icon.py", "icon.png")
    img.save(out_path, "PNG")
    print(f"wrote {out_path}")


if __name__ == "__main__":
    main()
