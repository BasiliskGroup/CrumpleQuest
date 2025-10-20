import pygame
import numpy as np
import mapbox_earcut as earcut
from graph import Graph
from geometry import ensure_winding
from funnel import get_portals, funnel
import time

# create polygon mesh
verts = np.array([
    # main poly
    [100, 100], [400, 100], [400, 300], [100, 300],
    
    # holes
    [200, 200], [300, 200], [300, 250], [200, 250],
    [200, 125], [300, 125], [275, 150],
], dtype=np.float32)
rings = np.array([4, 8, 11], dtype=np.uint32)

# check winding
ensure_winding(verts, 0, 4, clockwise=False)
ensure_winding(verts, 4, 8, clockwise=True)
ensure_winding(verts, 8, 11, clockwise=True)

# earcut
result = list(earcut.triangulate_int32(verts, rings))
verts = [tuple([float(p) for p in v]) for v in verts]
result = [int(r) for r in result]

# create graph
graph = Graph(result, verts)

# pygame
pygame.init()
screen = pygame.display.set_mode((500, 400))
pygame.display.set_caption("Earcut Triangulation Demo")
clock = pygame.time.Clock()

BG = (20, 20, 30)
POLY_OUTLINE = (255, 255, 0)
TRI_COLOR = (100, 200, 255)
VERTEX_COLOR = (255, 100, 100)
START_COLOR = (100, 255, 100)
END_COLOR = (255, 50, 50)

start_pos = [150, 150]
end_pos = [350, 250]
last_update = 0
update_interval = 0.25  # seconds

# path
a_star_result = []
final_path = []

running = True
while running:
    now = time.time()
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1: start_pos = np.array(event.pos, dtype=float)
            elif event.button == 3: end_pos = np.array(event.pos, dtype=float)

    # update A* every quarter second
    if now - last_update > update_interval:
        last_update = now
        a_star_indices = graph.a_star(start_pos, end_pos)
        a_star_result = [graph.tris[i].center for i in a_star_indices]
        portals = get_portals(graph.tris, a_star_indices)
        portals = [(start_pos, start_pos)] + portals + [(end_pos, end_pos)]
        final_path = funnel(portals, start_pos, end_pos)

    # draw
    screen.fill(BG)

    # Draw triangles
    for tri in graph.tris:
        pygame.draw.polygon(screen, TRI_COLOR, tri.verts, 0)
        pygame.draw.polygon(screen, POLY_OUTLINE, tri.verts, 1)

    # Draw polygon outlines
    pygame.draw.polygon(screen, POLY_OUTLINE, verts[:4], 2)
    pygame.draw.polygon(screen, POLY_OUTLINE, verts[4:8], 2)

    # Draw A* path
    for i in range(len(a_star_result) - 1):
        pygame.draw.line(screen, POLY_OUTLINE, a_star_result[i], a_star_result[i + 1], 2)
        
    # Draw funnel path
    for i in range(len(final_path) - 1):
        pygame.draw.line(screen, VERTEX_COLOR, final_path[i], final_path[i + 1], 3)

    # Draw start and end
    pygame.draw.circle(screen, START_COLOR, start_pos, 6)
    pygame.draw.circle(screen, END_COLOR, end_pos, 6)
    
    # Portals
    for left, right in portals:
        pygame.draw.circle(screen, (255, 100, 255), left, 4)
        pygame.draw.circle(screen, (255, 100, 0), right, 4)
        pygame.draw.line(screen, START_COLOR, left, right, 2)

    pygame.display.flip()
    clock.tick(60)

pygame.quit()
