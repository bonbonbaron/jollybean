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
    for s in map.sprites:
        if s.static == True:
            coll_grid[s.z][s.y >> 3][s.x >> 3] = s.type
    for e in map.exits:
        coll_grid[e.z][e.y >> 3][e.x >> 3] = e.type
        num_tiles_w = e.w >> 3
        num_tiles_h = e.h >> 3
        for i in range(num_tiles_w):
            for j in range(num_tiles_h):
                coll_grid[e.z][(e.y >> 3) + j][(e.x >> 3) + i] = 240 + e.id
        
