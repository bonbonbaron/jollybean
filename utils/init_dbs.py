from py_defs import *
import sys
from os import path

# Check arguments
args = sys.argv
if len(args) != 2:
    print("[init_dbs.py] intended use: python init_dbs.py [GAME_NAME]")
    quit()

# Check if game directory exists
game_name = sys.argv[2]
game_dir = "../%s"%game_name
if not path.exists("%s"%(game_dir):
    print("[init_dbs.py] Game directory %s/ does not exist."%(game_dir))
    quit()
if not path.exists("%s/dir/"%(game_dir)):
    print("[init_dbs.py] Game directory %s/ does not have a database subdirectory \"db\"."%(game_dir))
    quit()

# Check if db files in game directory exist. If they do, ignore them. If they don't, initialize them with the appropriate starter text.
db_files = ["media", "sprite", "scene", "audio", "img", "etc"]  # etc is for data that doesn't fit in any other category, like magic_spells. 
for db_file in db_files:
    db_fp = "%s/db/db_%s"%(game_dir, db_file)
    if path.exists(db_fp):
        continue
    f = open(db_fp, "w")
    f.write("%s\n%s%s%s%s%s"%(INCL_GE_H, START_ROW, END_ROW, TBL_DESC, START_TBL, END_TBL)
