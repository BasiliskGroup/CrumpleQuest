from geometry import is_ccw, triangle_area

EPSILON = 1e-3

def get_portals(nodes, idcs):
    portals = []
    for i in range(len(idcs) - 1):
        t1 = nodes[idcs[i]]

        edge_index = t1.adj[idcs[i + 1]]
        a = t1.verts[edge_index]
        b = t1.verts[(edge_index + 1) % 3]

        portals.append((a, b))

    return portals

def lenD2(a, b) -> float:
    return (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2

def funnel(portals, start, end):
    path = [start]
    
    if len(portals) == 0:
        path.append(end)
        return path
    
    apex_index = 0
    left_index = 0
    right_index = 0
    
    apex = start
    left = portals[0][0]
    right = portals[0][1]
    
    i = 1
    while i < len(portals):
        new_left = portals[i][0]
        new_right = portals[i][1]
        
        # update right
        if triangle_area(apex, right, new_right) <= 0:
            if lenD2(apex, right) < EPSILON or triangle_area(apex, left, new_right) > 0:
                
                right = new_right
                right_index = i
                
            else:
                
                # move apex left
                path.append(left)
                apex = left
                apex_index = left_index
                left = right = apex
                left_index = right_index = apex_index
                
                i = apex_index + 1
                continue
            
        # update left
        if triangle_area(apex, left, new_left) >= 0:
            if lenD2(apex, left) < EPSILON or triangle_area(apex, right, new_left) < 0:
                
                left = new_left
                left_index = i
                
            else:
                
                # apex moves right
                path.append(right)
                apex = right
                apex_index = right_index
                left = right = apex
                left_index = right_index = apex_index
                
                i = apex_index + 1
                continue
            
        i += 1
        
    path.append(end)
    return path