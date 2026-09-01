#!/usr/bin/env python3
"""Generate compact C++ geometry from the browser character study data."""

from __future__ import annotations

import hashlib
import json
import math
import pathlib
import re
from typing import Iterable, List, Sequence, Tuple


ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets" / "geometry-data.js"
OUTPUT = ROOT / "GrokStopWatch" / "GrokGeometry.generated.h"
POINT_SCALE = 64
BODY_POINT_COUNT = 96
Point = Tuple[float, float]


def load_geometry() -> Tuple[dict, str]:
    text = SOURCE.read_text(encoding="utf-8")
    match = re.search(r"window\.GROK_GEO = (\{.*?\});\n\n", text, re.S)
    if not match:
        raise RuntimeError(f"Could not find GROK_GEO JSON in {SOURCE}")
    payload = match.group(1)
    return json.loads(payload), hashlib.sha256(payload.encode()).hexdigest()


def cubic(p0: Point, p1: Point, p2: Point, p3: Point, t: float) -> Point:
    u = 1.0 - t
    return (
        u * u * u * p0[0]
        + 3.0 * u * u * t * p1[0]
        + 3.0 * u * t * t * p2[0]
        + t * t * t * p3[0],
        u * u * u * p0[1]
        + 3.0 * u * u * t * p1[1]
        + 3.0 * u * t * t * p2[1]
        + t * t * t * p3[1],
    )


def parse_blob_path(path: str) -> List[Point]:
    tokens = re.findall(r"[A-Za-z]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?", path)
    index = 0
    command = ""
    current = (0.0, 0.0)
    first = (0.0, 0.0)
    points: List[Point] = []

    def number() -> float:
        nonlocal index
        value = float(tokens[index])
        index += 1
        return value

    while index < len(tokens):
        if tokens[index].isalpha():
            command = tokens[index]
            index += 1

        if command == "M":
            current = (number(), number())
            first = current
            points.append(current)
            command = "L"
        elif command == "L":
            current = (number(), number())
            points.append(current)
        elif command == "C":
            p1 = (number(), number())
            p2 = (number(), number())
            p3 = (number(), number())
            p0 = current
            for step in range(1, 17):
                points.append(cubic(p0, p1, p2, p3, step / 16.0))
            current = p3
        elif command == "Z":
            if points[-1] != first:
                points.append(first)
            command = ""
        else:
            raise RuntimeError(f"Unsupported path command {command!r} in blob path")

    return points


def resample_closed(points: Sequence[Point], count: int) -> List[Point]:
    ring = list(points)
    if ring[0] != ring[-1]:
        ring.append(ring[0])

    lengths = [0.0]
    for a, b in zip(ring, ring[1:]):
        lengths.append(lengths[-1] + math.hypot(b[0] - a[0], b[1] - a[1]))

    total = lengths[-1]
    result: List[Point] = []
    segment = 0
    for sample in range(count):
        target = total * sample / count
        while segment + 1 < len(lengths) and lengths[segment + 1] < target:
            segment += 1
        a, b = ring[segment], ring[segment + 1]
        span = lengths[segment + 1] - lengths[segment]
        mix = 0.0 if span == 0.0 else (target - lengths[segment]) / span
        result.append((a[0] + (b[0] - a[0]) * mix, a[1] + (b[1] - a[1]) * mix))
    return result


def point_literal(point: Point) -> str:
    return "{%d,%d}" % (round(point[0] * POINT_SCALE), round(point[1] * POINT_SCALE))


def wrap_points(points: Iterable[Point], indent: str = "  ") -> str:
    values = [point_literal(point) for point in points]
    lines = []
    for start in range(0, len(values), 8):
        lines.append(indent + ",".join(values[start : start + 8]))
    return ",\n".join(lines)


def generate() -> str:
    geometry, source_hash = load_geometry()
    body = resample_closed(parse_blob_path(geometry["blobPath"]), BODY_POINT_COUNT)
    eyes = geometry["eyes"]
    if len(eyes) != 25 or any(len(pair) != 2 for pair in eyes):
        raise RuntimeError("Unexpected eye geometry topology")
    if any(len(poly) != 48 for pair in eyes for poly in pair):
        raise RuntimeError("All eye polygons must contain 48 points")

    eye_blocks = []
    for pair in eyes:
        left = wrap_points(pair[0], "      ")
        right = wrap_points(pair[1], "      ")
        eye_blocks.append("  {\n    {\n%s\n    },\n    {\n%s\n    }\n  }" % (left, right))

    center = geometry["Re"]
    eye_content = ",\n".join(eye_blocks)
    return f"""#pragma once

#include <stdint.h>

// Generated from assets/geometry-data.js.
// Source SHA-256: {source_hash}
namespace grok_geometry {{

struct Point16 {{
  int16_t x;
  int16_t y;
}};

static constexpr int kPointScale = {POINT_SCALE};
static constexpr float kSourceCenter = {center:.4f}f;
static constexpr uint16_t kBodyPointCount = {BODY_POINT_COUNT};
static constexpr uint8_t kEyeShapeCount = {len(eyes)};
static constexpr uint8_t kEyePointCount = 48;

static constexpr Point16 kBlobBody[kBodyPointCount] = {{
{wrap_points(body)}
}};

static constexpr Point16 kEyes[kEyeShapeCount][2][kEyePointCount] = {{
{eye_content}
}};

}}  // namespace grok_geometry
"""


def main() -> None:
    content = generate()
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    if OUTPUT.exists() and OUTPUT.read_text(encoding="utf-8") == content:
        print(f"Geometry is current: {OUTPUT}")
        return
    OUTPUT.write_text(content, encoding="utf-8")
    print(f"Generated: {OUTPUT}")


if __name__ == "__main__":
    main()
