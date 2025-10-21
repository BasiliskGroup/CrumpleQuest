import numpy as np

def sign(a, b, c) -> float:
    return (a[0] - c[0]) * (b[1] - c[1]) - (b[0] - c[0]) * (a[1] - c[1])

def point_in_triangle(p, v1, v2, v3) -> bool:
    d1 = sign(p, v1, v2)
    d2 = sign(p, v2, v3)
    d3 = sign(p, v3, v1)
    
    has_neg = d1 < 0 or d2 < 0 or d3 < 0
    has_pos = d1 > 0 or d2 > 0 or d3 > 0
    
    return not (has_neg and has_pos)

# checking functions (remove if we have math guarantees)
def polygon_area(points):
    """Shoelace formula to determine signed area (positive = CCW)."""
    x = points[:, 0]
    y = points[:, 1]
    return 0.5 * np.sum(x * np.roll(y, -1) - y * np.roll(x, -1))

def ensure_winding(verts, start, end, clockwise):
    ring = verts[start:end]
    area = polygon_area(ring)
    if (clockwise and area > 0) or (not clockwise and area < 0):
        verts[start:end] = np.flipud(ring)
        
# helper for funnel
def triangle_area(a, b, c):
    """2D cross product to check orientation"""
    area = (b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1])
    return -area # invert because we are y-down
        
def is_ccw(tri):
    """Check if triangle is counter-clockwise"""
    a, b, c = tri
    return triangle_area(a, b, c) > 0
