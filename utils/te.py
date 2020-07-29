import xml.etree.ElementTree as ET
import sys

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

class SceneTableRow:
    def __init__(self):
        self.type = 0
        self.num_layers = 0
        self.main_layer = 0
        self.layers = []
        self.num_sprites = 0
        self.sprite_ids = []
        self.sprite_positions = []
        self.sprite_orientations = []
        self.num_exits = 0
        self.exits = []
        self.num_songs = 0
        self.song_ids = []
        self.num_sounds = 0
        self.sound_ids = []
        self.exits = []
        self.collision_grid = []
        
args = sys.argv       
if len(args) < 3:
    print("Intended use of te.py: \"python te.py [GAME NAME] [SCENE FILE NAME (WITHOUT FILE EXTENSION)")
    quit()

game_name = args[1]
scene_name = args[2]

tree = ET.parse("../games/%s/scenes/%s.tmx"%(game_name, scene_name))
        
root = tree.getroot()

import csv
for scene_xml in root.iter("map"):
    scene = SceneTableRow()
    for l in scene_xml.iter("layer"):
        d = l.find("data").text  # ".text" is the value between the matching XML tags. 
        tile_indices = list(csv.reader(d))
        for row in tile_indices:
            if row.count("") > 0:
                row.remove("")
        scene.layers.append(tile_indices)  # contains all the rows of this layer
        scene.num_layers  += 1

    # Iter through scene's GE-specific properties
    for properties in scene_xml.iter("properties"):
        for prop in properties.iter("property"):
            if prop.attrib.get("name").lower() == "song":
                scene.song = prop.attrib.get("value").lower()
            elif prop.attrib.get("name").lower() == "type":
                scene.type = prop.attrib.get("value").lower()
            elif prop.attrib.get("name").lower() == "main_layer":
                scene.main_layer = int(prop.attrib.get("value").lower()) 
                if scene.main_layer >= scene.num_layers:
                    print("Index of main layer (%d) is too high for scene with %d layers. Exitting...")
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
                scene.sprites.append(sprite)
                scene.num_sprites += 1
                
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
                scene.exits.append(exit)

# Collision Grid
for l in scene.layers:
    coll_grid = [[[[0] * l.w] * l.h] * scene.num_layers]
    # Form a collision grid with sprites and exits. (Yes, exits are types of tiles.)
    for s in scene.sprites:
        if s.static == True:
            coll_grid[s.z][s.y >> 3][s.x >> 3] = s.type
    # TODO Study vim insert mode hotkeys
    # Exits are tiles whose types index into an array of Exit elements. Every scene needs this.
    for e in scene.exits:
        coll_grid[e.z][e.y >> 3][e.x >> 3] = e.type
        num_tiles_w = e.w >> 3
        num_tiles_h = e.h >> 3
        for i in range(num_tiles_w):
            for j in range(num_tiles_h):
                coll_grid[e.z][(e.y >> 3) + j][(e.x >> 3) + i] = 240 + e.id

# Write scene ID, scene type, num_sprites, sprite IDs, positions, and orientations, collision grid dimensions, and collision grid
"""
    typedef struct SceneTblRow {
	Uint8 num_sprites;
	Uint16 *sprite_ids;
	Vector2 *sprite_positions;
	Uint8 *sprite_orientations; /* Only as big as number of animated sprites. The pointer increments if sprite is animated. */
	Uint8 num_songs;
	Uint8 song_ids;
	Uint8 num_sounds;
	Uint8 sound_ids;
	/* If file I/O is the slowest part of the loading process, then let's load as much as we can at once: song ID, coll grid, tileset IDs, tilemaps, exits, etc. Somethings need to be dynamically populated, like the QT, etc. */
	MediaInfo media_info;
}
"""
# Scene type determines songs, sound effects, sprite behaviors, etc.
BYTEORDER = sys.byteorder
# TILEMAP
for i in range(len((scene.layers))):
    for j in range(len((scene.layers[i]))):
        for k in range(len((scene.layers[i][j]))):
            tilemap.layers[i][j][k] = int(tilemap.layers[i][j][k]).to_bytes(2, BYTEORDER)  # convert each tile index to two bytes
# Populate a bytearray. 
tilemap_bytearray = bytearray()
for i in range(len((tilemap.layers))):
    for j in range(len((tilemap.layers[i]))):
        for k in range(len((j))):
            tilemap_bytearray.extend(tilemap.layers[i][j][k])

# COLLISION GRID
# Write array of collision grid by row first, then all the rows in the coll grid's layer, then start over for the next layer and so forth.
# Convert collision grid's elements to Uint8s.
for i in range(scene.num_layers):
    for j in range(len((i))):
        for k in range(len((j))):
            coll_grid[i][j][k] = int(coll_grid[i][j][k]).to_bytes(1, BYTEORDER)  # Convert collision grid's elements to Uint8s.
# Populate a bytearray. 
collgrid_bytearray = bytearray()
for i in range(len((coll_grid))):
    for j in range(len((i))):
        for k in range(len((j))):
            collgrid_bytearray.extend(coll_grid[i][j][k])

# I want this to run directly from TileD. In order for that to work, I'm going to have to have it look inside the current directory (or design a directory structure standard for all GE games). This entails having to be inside the directory of the scenes in that game's parent directory, then accessing the config file like "../config" when running the script from within the working directory.

''' TODO: finish this list
game folder
    .sprites
    .levels <--- TileD will be in *here*, so access game title in parent directory.
    .sound
    .music
    meta
    data.bin <--- This may be the only product of the actual game development; GE just gets tacked on.
'''
DATA = "../games/%s/data.bin"%(game_name)
def get_file_sz(fp):
    f = open(fp)
    f.seek(0, 2)  # 2 is the end of the file... to get the filesize.
    return f.tell()

# Make bookmark with the end of the binary data file. Also, get size of what you're about to put in.
bookmark = get_file_sz(DATA)
sz = 0
for i in scene.layers:  # for each layer in collision grid...
    for j in i: # for each column in this layer...
        for k in j: for each row in this column...
            sz += (len(k) * 2)  # times two for two-byte indices in tilemap
for i in coll_grid:  # again, col grid is meant to be 3D for various layers. Not even necessarily parallax.
    for j in i:
        sz += len(j)

# data.bin
data_bytes = bytearray()
for i in tilemap:
    for j in i:
        for k in j:
            data_bytes.append(k) 
for i in coll_grid:
    for j in i:
        for k in j:
            data_bytes.append(k)  # I frickin' love Python. Look how easy this was...
import zlib as z
comp_data = z.compress(data_bytes)
f = open(DATA, "ab")
f.write(comp_data)
f.close()

# db.json
from json import JSONEncoder
class LemJSONEncoder(JSONEncoder):
    def default(self, o):
        return o.__dict__
json_data = LemJSONEncoder.encode(strow)
f = open("../games/%s/db.json"%(game_name), "a")
f.write(json_data)
f.close()

# db_posori.c
# db_scene.c
# db_exit.c
# db_posori.c
# db_sprite.c
# Something to consider: What other C files will I need to generate? Is there a way to factor common logic to common writer functions?
""" You'll have to make static arrays of the arrays so you don't have to allocate them on the heap:
    sprite ids
    sprite positions
    sprite orientation
    exits (even though they're tile-based, you need structs for trnasitions etc.)
    song_ids
    sound_ids


    When you write your database C file, you'll initialize everything in curly braces to keep it in one place. That way you don't have to call a function to initialize it. 

    It'll look something like this:

tbl_posori.c
    Uint8  *town1_ori = {0, 3, 2, 1, 3, 1, 2};
    Uint32 *town1_pos = {1543, 123, 12, 51, 61643, 23, 15};
    /* END_ROW_DEFS */

    /* /* Idx #*/ {Position array, Orientation array} */
    PosOriTblRow *posori_tbl = {
        /*   0 */ {town1_pos, town1_ori},
        /*   1 */ {town2_pos, town2_ori},
            ...
        /* 999 */ {final_pos, final_ori}
        /* END_TBL_DEF */
    };
    
tbl_scene.c
    #include "ge.h"

    /* Positions  and orientations */
    extern PosOriTblRow *tbl_posori;


    SceneTblRow *scene_tbl = {
        /*   0 */ {0, tbl_posori[0], 
"""

"""
typedef struct SceneTblRow {
	Uint8    num_sprites;
	Uint8   *sprite_orientations; /* Only as big as number of animated sprites. The pointer increments if sprite is animated. */
	Uint8    num_exits;
	Uint8    num_songs;
	Uint8    song_ids;  /* TODO: implement songs so you can put them in the Scene struct */
	Uint8    num_sounds;
	Uint8    sound_ids;  /* TODO: implement sound so you can put it in the Scene struct */
	Uint16  *sprite_ids;
	Vector2 *sprite_positions;
	Exit    *exits; /* This gets indexed by colliding into a tile whose type contains this index. */
	/* If file I/O is the slowest part of the loading process, then let's load as much as we can at once: song ID, coll grid, tileset IDs, tilemaps, exits, etc. Some things need to be dynamically populated, like the QT, etc. */
	MediaInfo media_info;
} SceneTblRow; 
"""
