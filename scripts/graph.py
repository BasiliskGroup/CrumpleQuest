import heapq
import math
from geometry import point_in_triangle, is_ccw

class AStarNode():
    
    def __init__(self, adj: list[dict[int : tuple[int, int]]], verts: list[tuple[float, float]], center: tuple[float, float]) -> None:
        self.adj = adj
        self.verts = verts if is_ccw(verts) else verts[::-1]
        self.center = center
        self.reset()
        
    def reset(self):
        self.g = 0
        self.f = 0
        
    def __getitem__(self, key: int) -> tuple[float, float]:
        return (self.verts[key], self.verts[(key + 1) % 3])

class Graph():
    
    def __init__(self, inds: list[int], pts: list[tuple[float, float]]) -> None:
        
        # geometry
        self.pts = pts
        centers: list[tuple[float, float]] = []
        
        for i in range(len(inds) // 3):
            centers.append((
                (pts[inds[3 * i + 0]][0] + pts[inds[3 * i + 1]][0] + pts[inds[3 * i + 2]][0]) / 3,
                (pts[inds[3 * i + 0]][1] + pts[inds[3 * i + 1]][1] + pts[inds[3 * i + 2]][1]) / 3,
            ))
            
        # add triangles
        self.tris: list[AStarNode] = []
        for i in range(len(centers)):
            self.tris.append(AStarNode([], [self.pts[j] for j in inds[3 * i : 3 * (i + 1)]], centers[i]))
            
        # tie triangles together
        adj_lists: list[list[int]] = self.build_triangle_graph()
        for i, (adj, tri) in enumerate(zip(adj_lists, self.tris)):
            tri.adj = adj
            print(f"Triangle {i} adjacent to {sorted(adj)}")
            
        # A* variables
        self.clear_a_data()
        
    def build_triangle_graph(self):
        edge_map = {}  # maps edge (v1, v2) -> (tri_id, edge_index)
        adjacency = [{} for _ in range(len(self.tris))]

        for tri_id, tri in enumerate(self.tris):
            # Each triangle has 3 edges: (0,1), (1,2), (2,0)
            for edge_index in range(3):
                a = tri.verts[edge_index]
                b = tri.verts[(edge_index + 1) % 3]
                reverse_edge = (b, a)

                if reverse_edge not in edge_map:
                    edge_map[(a, b)] = (tri_id, edge_index)
                    continue

                # Found reverse edge -> triangles are adjacent
                other_id, other_edge_index = edge_map.pop(reverse_edge)
                adjacency[tri_id][other_id] = edge_index
                adjacency[other_id][tri_id] = other_edge_index

        return adjacency
        
    def heuristic(self, tri: int, goal: int) -> float:
        center = self.tris[tri].center
        goal = self.tris[goal].center
        dx = center[0] - goal[0]
        dy = center[1] - goal[1]
        return math.sqrt(dx * dx + dy * dy)
    
    def pos_to_tri(self, pos) -> int:
        if isinstance(pos, int):
            return pos
        
        for i, tri in enumerate(self.tris):
            if point_in_triangle(pos, *tri.verts):
                return i
                
        return -1
    
    def clear_a_data(self) -> None:
        self.open_set = []
        self.open_lookup = {}
        self.closed_set = set()
        self.came_from = {}
        
        for tri in self.tris:
            tri.reset()
    
    def a_star(self, start, goal):
        # convert positions to nodes if necessary
        start = self.pos_to_tri(start)
        goal = self.pos_to_tri(goal)
        
        if start < 0 or goal < 0:
            print('path not found')
            return []
        
        self.clear_a_data()
        
        # run A*
        start_node = self.tris[start]
        start_node.g = 0
        start_node.f = self.heuristic(start, goal)
        
        # push start into open heap
        heapq.heappush(self.open_set, (start_node.f, start))
        self.open_lookup[start] = start_node
        
        while len(self.open_set) > 0:
            # pop lowest f-score
            _, cur_idx = heapq.heappop(self.open_set)
            cur: AStarNode = self.tris[cur_idx]
            self.open_lookup.pop(cur_idx, None)
            
            # check if we are at goal
            if cur_idx == goal:
                
                # reconstruct path
                path = [cur_idx]
                while cur_idx in self.came_from:
                    cur_idx = self.came_from[cur_idx]
                    path.append(cur_idx)
                path.reverse()
                return path
            
            # mark as visited
            self.closed_set.add(cur_idx)
            
            # interate adjacents
            for adj_idx in cur.adj.keys():
                if adj_idx in self.closed_set:
                    continue
                
                adj: AStarNode = self.tris[adj_idx]
                
                # cost from current to neighbor = distance between triangle centers
                tent_g = cur.g + math.dist(cur.center, adj.center)
                
                # if neighbor not in open set or found a better path
                if adj_idx not in self.open_lookup or tent_g < adj.g:
                    self.came_from[adj_idx] = cur_idx
                    adj.g = tent_g
                    adj.f = tent_g + self.heuristic(adj_idx, goal)
                    
                    if adj_idx not in self.open_lookup:
                        heapq.heappush(self.open_set, (adj.f, adj_idx))
                        self.open_lookup[adj_idx] = adj
        
        # no path found
        print('path not found')
        return []