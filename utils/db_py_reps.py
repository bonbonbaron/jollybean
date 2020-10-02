from py_defs import *
from utils import *

class Sprite:
    def __init__(self, name):
        self.name = name
        self.type = SPRITE
        #####################  <~~ separates JB-specific attributes from those only needed by the database setup system
        self.onscreen = False
        self.reaction_bookmark = 0
        self.id = 0
        self.type = 0
        self.species = 0
        self.surface = NULL
        self.rect = None
        self.blit_coords = None
        self.react_seq_grp = NULL

class Scene:
    def __init__(self, name):
        self.name = name
        self.type = SCENE
        #####################
        self.sprites = [];  
        self.num_sprites = 0;
        self.type = 0;  
        self.bg_idx = 0;
        self.exits = [];
        self.backgrounds = [];
        self.signals = [];
        self.camera = None;
        self.coll_quadtree = NULL;
        self.coll_grid = NULL


