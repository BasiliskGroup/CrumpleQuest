import numpy as np
import random


WIDTH = 13
MIN = 17
MEAN = 25
STDEV = 3

TEMP_REDUCT = 0.95

# enum
SPAWN = 0
BASIC = 1
BOSS = 2

play = [[-1 for _ in range(WIDTH)] for _ in range(WIDTH)]
temperature = [[0 for _ in range(WIDTH)] for _ in range(WIDTH)]
distance = [[-1 for _ in range(WIDTH)] for _ in range(WIDTH)]

# start in the center
center = WIDTH // 2
valids = {}

# colors
sym_map = { 
    -1 : '  ', 
    0 : "\033[38;2;128;255;0m██\033[0m", 
    1: "██", 
    2: "\033[38;2;180;0;0m██\033[0m"
}

temp_blocks = {
    -1: "\033[38;2;255;0;0m██\033[0m",      # Bright red
    0: "██", 
    1: "\033[38;2;128;255;0m██\033[0m",    # Yellow-green
    2: "\033[38;2;200;200;0m██\033[0m",    # Yellow
    3: "\033[38;2;255;128;0m██\033[0m",    # Orange
    4: "\033[38;2;255;64;0m██\033[0m",     # Reddish orange
    5: "\033[38;2;255;0;0m██\033[0m",      # Red
    6: "\033[38;2;180;0;0m██\033[0m"       # Deep red
}

# helper functions
def in_range(x: int, y: int) -> bool:
    return 0 <= x < WIDTH and 0 <= y < WIDTH

def get_around(x : int, y : int) -> list[tuple[int, int]]:
    around = []
    if in_range(x - 1, y): around.append((x - 1, y))
    if in_range(x + 1, y): around.append((x + 1, y))
    if in_range(x, y - 1): around.append((x, y - 1))
    if in_range(x, y + 1): around.append((x, y + 1))
    return around

def add(x: int, y: int, type: int) -> None:
    play[x][y] = type
    temperature[x][y] = -1 # flag for unusable
    
    for adj in get_around(x, y):
        dx, dy = adj
        distance[x][y] = max(distance[x][y], distance[dx][dy] + 1)
        if temperature[dx][dy] == -1: continue
            
        temperature[dx][dy] += 1
        valids[adj] = temperature[dx][dy]
        
def print_map_row(i: int) -> None:
    # normalize distance map
    high = [max(d) for d in distance]
    high = max(high) / 5
    
    norm_dist = [[i // high + 1 for i in d] for d in distance]
    
    print('[', 
        ''.join([sym_map[j] for j in play[i]]), ']   [', 
        ''.join([temp_blocks[j] for j in temperature[i]]), ']   [', 
        ''.join([temp_blocks[j] for j in norm_dist[i]]), 
    ']')
        
# add spawn room to sets
add(center, center, SPAWN)

for i in range(1, max(MIN, int(np.random.normal(loc=MEAN, scale=STDEV, size=1).item()))):
    
    # select next room based on temperature
    adjs = list(zip(valids.values(), valids.keys()))
    adjs.sort(key = lambda p: p[0])
    x, y = adjs[0][1]
    
    prob = 1
    j = 1
    while random.uniform(0, 1) < prob and j < len(adjs):
        x, y = adjs[j][1]
        j += 1
        prob *= TEMP_REDUCT
    
    # add room
    del valids[adjs[j - 1][1]]
    add(x, y, BASIC)
    
# add boss room
high = (center, center)
for x in range(WIDTH):
    for y in range(WIDTH):
        if distance[x][y] < distance[high[0]][high[1]]: continue
        high = (x, y)
        
play[high[0]][high[1]] = BOSS
        
# draw maps
for j in range(WIDTH):
    print_map_row(j)