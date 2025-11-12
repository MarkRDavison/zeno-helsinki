#!/usr/bin/env python3

import argparse
import sys
from pathlib import Path
from PIL import Image

# Cube face layout in the 4x3 grid
FACE_POSITIONS = {
    "top": (1, 0),
    "left": (0, 1),
    "front": (1, 1),
    "right": (2, 1),
    "back": (3, 1),
    "bottom": (1, 2),
}

def main():
    parser = argparse.ArgumentParser(description="Extract cube map faces from a single image.")
    parser.add_argument("input", type=Path, help="Path to the input cubemap image")
    parser.add_argument("--prefix", type=str, default="", help="Prefix for output filenames")
    parser.add_argument("--face-size", type=int, help="Optional face size (overrides automatic calculation)")
    parser.add_argument("--format", type=str, help="Optional output image format (default: same as input)")
    parser.add_argument("--force", action="store_true", help="Overwrite existing output files")
    args = parser.parse_args()

    if not args.input.exists():
        print(f"Input file {args.input} does not exist.", file=sys.stderr)
        sys.exit(1)

    img = Image.open(args.input)
    img_width, img_height = img.size

    # Determine face size
    if args.face_size:
        face_size = args.face_size
    else:
        # auto calculate
        face_width = img_width // 4
        face_height = img_height // 3
        if face_width != face_height:
            raise ValueError(f"Calculated face dimensions are not square: {face_width}x{face_height}")
        face_size = face_width

    # Determine output format
    out_format = args.format.upper() if args.format else img.format
    if out_format is None:
        out_format = "PNG"  # fallback

    for face_name, (grid_x, grid_y) in FACE_POSITIONS.items():
        left = grid_x * face_size
        upper = grid_y * face_size
        right = left + face_size
        lower = upper + face_size
        face_img = img.crop((left, upper, right, lower))

        filename = f"{args.prefix}{face_name}.{out_format.lower()}"
        out_path = Path(filename)
        if out_path.exists() and not args.force:
            print(f"Skipping {filename} (exists, use --force to overwrite)")
            continue

        face_img.save(out_path, format=out_format)
        print(f"Saved {filename}")

if __name__ == "__main__":
    main()
