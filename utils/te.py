import xml.etree.ElementTree as ET
import sys
#TODO make argc count, then loop through all map files 
class Sprite:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.z = 0
        self.orientation = 0
        self.static = False

class Exit:
    def __init__(self):
        self.id = 0
        self.x = 0
        self.y = 0
        self.z = 0
        self.tgt_scene = None
        self.transition = None
        self.entry_point = 0

class Layer:
    def __init__(self):
        self.w = 0
        self.h = 0
        self.looping = False
        self.vx = 0
        self.vy = 0

class Map:
    def __init__(self):
        self.song = None
        self.type = None
        self.num_layers = 0
        self.main_layer = 0
        self.layers = []
        self.exits = []
        self.collision_grid = []
        
tree = ET.parse("%s.tmx"%(sys.argv[1]))
        
root = tree.getroot()

import csv
for map_xml in root.iter("map"):
    map = Map()
    for l in map_xml.iter("layer"):
        d = l.find("data").text
        tile_indices = list(csv.reader(d))
        for row in tile_indices:
            if row.count("") > 0:
                row.remove("")
        map.layers.append(tile_indices)
        map.num_layers  += 1

    # Iter through map's GE-specific properties
    for properties in map_xml.iter("properties"):
        for prop in properties.iter("property"):
            if prop.attrib.get("name").lower() == "song"
                map.song = prop.attrib.get("value").lower()
            elif prop.attrib.get("name").lower() == "type":
                map.type = prop.attrib.get("value").lower()
            elif prop.attrib.get("name").lower() == "main_layer":
                map.main_layer = int(prop.attrib.get("value").lower()) 
                if map.main_layer >= map.num_layers:
                    print("Index of main layer (%d) is too high for map with %d layers. Exitting...")
                    quit()

for layer_xml in root.iter("layer"):
    layer = Layer()
    layer.x = layer_xml.attrib.get("width")
    layer.y = layer_xml.attrib.get("height")
    for properties in layer_xml.iter("properties"):
        for prop in properties.iter("property"):
            if prop.attrib.get("name").lower() == "looping":
                layer.looping = prop.attrib.get("value")
            if prop.attrib.get("name").lower() == "vx":
                layer.vx = prop.attrib.get("value")
            if prop.attrib.get("name").lower() == "vy":
                layer.vy = prop.attrib.get("value").lower()
    
for objectgroup in root.iter("objectgroup"): 
   # Sprites
    if (objectgroup.attrib["name"].lower() == "sprites"):
        for obj in objectgroup:
            sprite = Sprite()
            sprite.x = int(obj.attrib.get("x")) & ~7
            sprite.y = int(obj.attrib.get("y")) & ~7
            for props in obj:
                for prop in props:
                    if prop.attrib.get("name").lower() == "orientation":
                        sprite.orientation = prop.attrib.get("value")
                    elif prop.attrib.get("name").lower() == "static":
                        sprite.static = prop.attrib.get("value")
                    elif prop.attrib.get("name").lower() == "z":
                        sprite.z = int(prop.attrib.get("value"))
                map.sprites.append(sprite)
    # Exits
    if (objectgroup.attrib["name"].lower() == "exits"):
        exit_counter = 0
        for obj in objectgroup:
            exit = Exit()
            exit.id - exit_counter
            exit_counter += 1
            exit.x = int(obj.attrib.get("x")) & ~7
            exit.y = int(obj.attrib.get("y")) & ~7
            for props in obj:
                for prop in props:
                    if prop.attrib.get("name").lower() == "tgt_scene":
                        exit.tgt_scene = prop.attrib.get("value")
                    elif prop.attrib.get("name").lower() == "transition":
                        exit.transition = prop.attrib.get("value")
                    elif prop.attrib.get("name").lower() == "entry_point":
                        exit.entry_point = prop.attrib.get("value")
                    elif prop.attrib.get("name").lower() == "z":
                        exit.z = int(prop.attrib.get("value"))
                map.exits.append(exit)

# Collision Grid
for l in map.layers:
    coll_grid = [[[[0] * l.w] * l.h] * map.num_layers]
    # Form a collision grid with sprites and exits. (Yes, exits are types of tiles.)
    for s in map.sprites:
        if s.static == True:
            coll_grid[s.z][s.y >> 3][s.x >> 3] = s.type
    # TODO Study vim insert mode hotkeys
    # Exits are tiles whose types index into an array of Exit elements. Every scene needs this.
    for e in map.exits:
        coll_grid[e.z][e.y >> 3][e.x >> 3] = e.type
        num_tiles_w = e.w >> 3
        num_tiles_h = e.h >> 3
        for i in range(num_tiles_w):
            for j in range(num_tiles_h):
                coll_grid[e.z][(e.y >> 3) + j][(e.x >> 3) + i] = 240 + e.id

# Write scene ID, scene type, num_sprites, sprite IDs, positions, and orientations, collision grid dimensions, and collision grid
# Scene type determines songs, sound effects, sprite behaviors, etc.
# Write array of collision grid by row first, then all the rows in the coll grid's layer, then start over for the next layer and so forth.
# Convert collision grid's elements to Uint8s.
for i in range(len((coll_grid))):
    for j in range(len((i))):
        for k in range(len((j))):
            coll_grid[i][j][k] = int(coll_grid[i][j][k]).to_bytes(1, "big") 
# Populate a bytearray. 
collgrid_bytearray = bytearray()
for i in range(len((coll_grid))):
    for j in range(len((i))):
        for k in range(len((j))):
            collgrid_bytearray.extend(coll_grid[i][j][k])

# I want this to run directly from TileD. In order for that to work, I'm going to have to have it look inside the current directory (or design a directory structure standard for all GE games). 
''' TODO: finish this list
game folder
    .sprites
    .levels <--- TileD will be in *here*, so access game title in parent directory.
    .sound
    .music
    meta
    data.bin <--- This may be the only product of the actual game development; GE just gets tacked on.
'''
import sys
META = "../meta"
DATA = "../data.bin"
BYTEORDER = sys.byteorder
def get_file_sz(fp):
    f = open(fp)
    f.seek(0, 2)
    return f.tell()

# Make bookmark with the end of the binary data file. Also, get size of what you're about to put in.
bookmark = get_file_sz(DATA)
sz = 0
for i in coll_grid:  # again, col grid is meant to be 3D for various layers. Not even necessarily parallax.
    for j in i:
        sz += len(j)

# Make all tiles two bytes
data_bytes = bytearray()
for i in coll_grid:     # layers
    for j in i:         # rows 
        for k in j:     # individual elements
            data_bytes.append(int(k).to_bytes(2, BYTEORDER))
            

# Compress with zlib


# Read in compressed file and Append to the end of the binary data file.
f = open(DATA, "ab")
f.write()
"""
    if tilemap is not None:
        tm_data = [0, 0, 0, 0]
        tm_data[IDX_LEN] = 2 * tilemap.shape[0] * tilemap.shape[1]  # USHORT data type
        tm_data[IDX_W]   = tilemap.shape[1]
        tm_data[IDX_H]   = tilemap.shape[0]
        temp = list(tilemap.flatten())
        temp = [int(t).to_bytes(2, BYTEORDER) for t in temp]
        tm_array = bytearray()
        for t in temp:
            tm_array.extend(t)
        temp = [int(t).to_bytes(2, BYTEORDER) for t in temp]
"""

