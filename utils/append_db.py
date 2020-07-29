# NOTE: you have to use Python 3 to preserve ordering of class attributes when writing to the C database files.
from py_defs import *
import sys
from os import path

# Check arguments
args = sys.argv
if len(args) != 4:
    print("[append_db.py] intended use:\n\t python append_db.py [game_name] [scene | sprite | audio | text | img] [object name]\n ... where object_name is the name of the game object you're appending without the extension")
    quit()

args[3] = args[3].lower()
if args[3] not in DBS:
    print("[append_db.py] intended use:\n\t python append_db.py [game_name] [scene | sprite | audio | text | img] [object name]\n ... where object_name is the name of the game object you're appending without the extension")
    quit()

# Check if game directory exists
game_name = sys.argv[1]
obj_type = sys.argv[2]
obj_name = sys.argv[3]
obj_dir  = "../games/%s/%s"%(game_name, obj_name)
obj_db   = "../games/%s/db_%s"%(game_name, obj_type)
obj_json = "../games/%s/db_%s.json"%(game_name, obj_type)

if not path.exists(obj_dir):
    print("[append_db.py] %s directory %s/ does not exist."%(obj_type, obj_dir))
    quit()

# Open database file for this object type.
f = open(obj_db, "r")
db_lines = f.readlines()
f.close()

# Get locations of where to write for the individual row. 
idx_row_start = db_lines.index(START_ROW) + 1
idx_row_end   = db_lines.index(END_ROW) + 1
idx_tbl_start = db_lines.index(START_TBL) + 1
idx_tbl_end   = db_lines.index(END_TBL) + 1

# For updates (not inserts), you'll have to get the index by looking it up in the JSON table. Seems like this oculd line up with the enum in C by grabbing the index and subtracting that by the index of /* ENUM_START */. 
f = open(obj_db, "w")
for dl in db_lines:  # data_lines was created above
    f.write(dl)
f.close()

# params: data and target db
def insert(data, tgt_tbl):
  pass;

def update(data, tgt_tbl):
  pass;

def delete(data, tgt_tbl):
  pass;

'''
Okay. This is the part where I have to design the software for different kinds of databases. How to make this as easy as possible? And how to make all the data flow through the same gates so that, if something's wrong with all of them, it can all be fixed in one place? 

To completely understand the problem, I'm going to walk through it first at a macro level, then a micro level for each macro.

MACRO LEVEL:

    Media application > Media-specific data genereator w/ object representation > Insert()/Update() > db_*.c
    
    There are a few steps to creating a database file for any datatype:
        1) Identify and Create all the constants
          a) Python representation of everything in the table records; composite items are their own subclasses prepended by "&".
          b) Here's the challenge: The lowest level composites must come before the ones that contain them. The easiest way to avoid a HUGE mess of reordering the lines properly-- especially in the event that data needs to be edited-- is by having them in their own separate files. That may be a lot of files, but think of it from a maintainability standpoint: If I need to udpate Bowser's attack damage, I'll simply go to battle_stats_db.c and find bowser_battle_stats.

          But from an organizational standpoint, something's gotta give. Subdirectories need to contain all the elements of a composite datatype. 
          c) If type(obj) == "instance", then go through its contents. If an instance has multiple elements but no composites, it's a const. If it has any composites, then it's its own table. Kinda like this:
            Sprites/
                sprites_db.c  <-- This would contain the metadata for reading the images from the data.bin file, indexable by ID.
                sprite_attrs/  <-- everything associated with sprites... except their placement in levels.
                    [sprite name 1]/
                        [sprite_name_1]_battle_stats.c  <- Yeah... This is the best way to do it for maintainability.
                        [sprite_name_1]_magic_spells.c
                
            The above design carries over well to db_scene.c too. It can go scene_attrs/... with exit and posori for each scene.


          d) Using each object's __dict__ attribute, you can loop through them in order using [class instance].__dict__[elem name].
             Begin the loop with {, then write every value of the __dict__, and finish with }.

             EXAMPLE: So the above'll go something like this:    
                ..., {4, 45, 8, 12, &ben_battle_stats, "aeiou"}, ...

          e) If type(obj) == "instance", then go through its contents. If an instance has multiple elements but no composites, it's a const. If it has any composites, then it's its own table.   
        2) Create all the sub-tables
        3) Create the tables
        4) You'll need a dict for the consts and a list for the rows and tables.
        

    TODO: Come up with a way to prevent table row-shifting from breaking the indexes in dependent tables.
        1) Need a name in the python objects. All of them. Error out if any is missing.
        2) Put name in comment next to table row.
        3) Every time you update or write to a table, read all the commented enum names into a list and regenerate the enum in the game's H file.
        4) Every index MUST be an enum!!! Or else, it risks breaking both the table and my heart.

    Here are the macro categories. Each sublevel indicates a jagged array. A jagged array, for cleaniness, necessarily has to be its own table with the pointed-at arrays initialized separately. Those individual initializations ought to be const. I think the compiler can speed up accessing using const-indirection from the symbol table than having to load a variable from memory. "*" denotes a jagged array; single variable otherwise.

    NOTE: You never have to put an object's ID in its table. That's simply what you index it by. It'll be enum'd in another C file somewhere. Maybe the best place to do that, since these will be called in several different places (or will they?), is in the game's H file. For example, if I'm loading a Scene, I will be referencing the next scene by its enum'd ID in... No, I don't need it! The Exit object contains that inside itself.
"scene", "sprite", "audio", "text", "image"
scene, posori, exit, sprite, sound, text, menu, motion, animation, reaction,  

======================================================================
LEGEND:
(P) = "Pointer, NULL if not loaded, reusable otherwise"
(O) = "Its Own Table, but indexed by another" <-- Every row of a table has to be the same length, hence separation of the jagged portions!!!
(C) = Allocated in const variable inside same file as this object.
[nothing] = Top Level Table that probably accesses others.

***DATABASE RULE OF THUMB: If it's a sub-array of a jagged array, then it is a const array. If it's a container of jagged arrays, then it's a table. Every row of a table has to be the same length, hence separation of the jagged portions!!!***
======================================================================

  Scene
    type
    num_sprites
    num_bgs
    num_exits
    num_sounds
    (C)Posori
    (C)Sprite IDs
    (C)BG IDs
    (C)Sound IDs
    (C)Text IDs
    (C)Menu IDs  <-- This is a whole can of worms I haven't opened yet.
    (C)Exits <-- (C), because you can do {{134134, 12}, {235, 235235}, ...} in a constant variable.
  Exit
    Transition Type
    Tgt Scene ID
  Sprite
    bpp
    num_colors
    type
    rect_w
    rect_h
    surface_w
    surface_h
    media_info
    (C)tilemap
    (C)colors
    (P)surface_ptr

  ***BELOW: Reactions are more complex: rsgo > rsg > rs > r, r, r, ...***  
  Reaction Sequence Group Options
    num_react_seq_options
    (P)react_seq_grp_option
    (O)react_seq_grp_options 
  (O)Reaction Sequence Groups <-- Sprite gets this @ loadtime, not tbl
    priority
    num_react_seqs
    (C)*Triggers  <-- each trigger kicks off an entire sequence
    (O)**ReactSequences <-- confusing: array of pointers to rs's, each of which is an element of a jagged array
  (O)Reaction Sequences
    num_reactions
    repeat
    (C)*react_func_ptr
    

TODO: write tools to query human-readable Sprite locations, reactions and their triggers, etc. with an easy-to-use command-line interface. Can also query scenes, who's in them, where they are, how many of each sprite, etc.
'''

