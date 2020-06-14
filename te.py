import xml.etree.ElementTree as ET
import sys
#TODO make argc count, then loop through all map files 
class Sprite:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.orientation = 0
        self.static = False

class Exit:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.tgt_scene = None
        self.transition = None
        self.entry_point = 0

exits = []
sprites = []

tree = ET.parse("%s.tmx"%(sys.argv[1]))
root = tree.getroot()
for objectgroup in root.iter("objectgroup"): 
    # Collision Grid
    #TODO
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
                sprites.append(sprite)
    # Exits
    if (objectgroup.attrib["name"].lower() == "exits"):
        for obj in objectgroup:
            exit = Exit()
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
                exits.append(exit)
print("Collision Grid")
#TODO 

print("Sprites")
for s in sprites:
    print(s.x)
    print(s.y)
    print(s.orientation)
    print(s.static)
    print("\n")
    
print("Exits")
for e in exits:
    print(e.x)
    print(e.y)
    print(e.tgt_scene)
    print(e.transition)
    print(e.entry_point)
    print("\n")
