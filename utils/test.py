from py_defs import *
import sys
from os import path

# Check arguments
args = sys.argv
if len(args) != 2:
    print("[append_db.py] intended use: python append_db.py [GAME_NAME]")
    quit()

# Check if game directory exists
scene_name = sys.argv[2]
scene_dir = "../%s"%scene_name
if not path.exists("%s"%(scene_dir)):
    print("[append_db.py] Scene directory %s/ does not exist."%(scene_dir))
    quit()

f = open("testdata", "r")
data_lines = f.readlines()
f.close()

idx_row_start = data_lines.index(START_ROW)
idx_row_end   = data_lines.index(END_ROW)
idx_tbl_start = data_lines.index(START_TBL)
idx_tbl_end   = data_lines.index(END_TBL)

f = open("testdata", "w")
for dl in data_lines:
    f.write(dl)
f.close()
