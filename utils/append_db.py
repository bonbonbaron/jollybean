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
obj_dir  = "../games/%s/%s"%(game_name, obj_type)
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
idx_row_end   = db_lines.index(END_ROW)   + 1
idx_tbl_start = db_lines.index(START_TBL) + 1
idx_tbl_end   = db_lines.index(END_TBL)   + 1

# For updates (not inserts), you'll have to get the index by looking it up in the JSON table. Seems like this oculd line up with the enum in C by grabbing the index and subtracting that by the index of /* ENUM_START */. 
f = open(obj_db, "w")
for dl in db_lines:  # data_lines was created above
  f.write(dl)
f.close()

# CRUD CONSTANTS
MERGE  = 1
DELETE = 2

def crud(crud_type, python_obj, attr_name=None):
  obj_dir = "%s%s%s"%(obj_dir, SEP, python_obj.type)
  if attr_name is None:
    filename = "%s%s%s_db.c"%(obj_dir, SEP, python_obj.name)
  else:
    filename = "%s%s%s%sattr%s%s_db.c"%(obj_dir, SEP, python_obj.name, SEP, SEP, attr_name)
    
  # Insert/update overarching table
  f = open(filename, "r")
  # Trusting that Python's class data aligns with C's struct data...
  obj_exists_in_db = False
  file_lines = f.readlines()
  f.close()
  for line_idx in range(len(file_lines)):
    if python_obj.name in file_lines(line_idx):
      before_obj_lines = file_lines[:line_idx]
      if (line_idx != len(file_lines) - 1):
        after_obj_lines  = file_lines[(line_idx + 1):]
      else:
        after_obj_lines = ""
      obj_exists_in_db = True
    break
  # If element is found in DB, then copy the lines before and after. Then paste the before-lines, the new lines, and after.
  if obj_exists_in_db:
    # If any of the object's attributes are structs, they must go into another DB in current directory's child folder.
    for obj_attr_name in python_obj.__dict__.keys():
      # If this object is a Python class (struct), recurse for the next directory.
      if type(obj_attr_name) == "instance":
        crud(crud_type, python_obj, eval("%s.%s")%(python_obj, attr_name))
      else:
        f = open(filename, "w")
        f.write(before_obj_lines)
        if crud_type == MERGE:
          f.write("\t%s,\n"%(python_obj.__dict__.keysobj_attr_name]))
        f.write(after_obj_lines)
        f.close()


# merge() both inserts and updates, since we're breaking everything up into separate files.
def merge(python_obj, attr_name=None):
  crud(MERGE, python_obj, attr_name)


def delete(python_obj, attr_name=None):
  crud(DELETE, python_obj, attr_name)

