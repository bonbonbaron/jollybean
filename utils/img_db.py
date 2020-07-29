from py_defs import *
import sys, os
from img import compress_sprite_imgs

# argv[0]: python file
# argv[1]: game name

if len(sys.argv != 2):
    print("[img_db.py] intended use: python img_db.py [src file.png]\nExitting...")
    quit()

#game_nm = sys.argv[1]
src_file = sys.argv[1]
pwd = os.getcwd()

# Since we're running this from Aseprite, we don't know what game we're doing. So we see what grandparent (lol) directory we're in.
# TODO: make a more robust way to do this later on... But please don't clunkify everything with config files.
game_nm = os.getcwd().split(SEP)[-3]  # .../ge/games/[GAME NAME]/img/sprite/
img_type = os.getcwd().split(SEP)[-1] # .../ge/games/[GAME NAME]/img/[sprite | bg]/
img_nm = src_file.split(".")[0]

if img_type not in ["bg", "sprite"]:
    print("[img_db.py] Only run img_db.py from the game's img/sprite or img/bg folders. Quitting...")
    quit()

# See if the name of the current BG or Sprite is a folder name in this directory. If not, make the directory and initialize the files.
if not os.getcwd()[-1] == img_nm:
    print("Make a directory for your bg or sprite in [game dir]/img/[bg | sprite]/. Example: If you're making a sprite named Mario, make a folder called /home/bonbonbaron/ge/hack/ge/master/ge/Mario3/img/sprite/mario/. Then run this script from inside there. \nExitting...")
    quit()

#See if last_modified_tm file exists. If not, write it. If so, only redo the sprite's image if it's been modified since last time.
if "last_modified_tm" not in os.listdir():
    # compress_sprite_imgs(directory, sprite_name):
    compress_sprite_imgs(os.getcwd(), img_nm)
    last_mod_tm = os.stat("./%s.bin"%(img_nm)).st_mtime
    f = open("last_proc_tm", "w")
    f.write(last_mod_tm)
    f.close()
else:
    f = open("last_proc_tm", "r")
    last_proc_tm = f.read()
    f.close()
    last_mod_tm = os.stat("./%s.bin"%(img_nm)).st_mtime
    if int(last_proc_tm) > last_mod_tm:
        compress_sprite_imgs(os.getcwd(), img_nm)
    
from db_py_reps import Sprite    
    


''' TODO
    ----
    Write sprite row to a local file here too.
        What's the sprite row? This is where all the heavy database-writing design work begins.
    Parse filename for enum of anim name, if applicable.
    If this is new, you can just append to the master table. Otherwise, you have to update all the successive sprites' offsets in the image file.
