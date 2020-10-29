# NOTE: you have to use Python 3 to preserve ordering of class attributes when writing to the C database files.
from py_defs import *
import sys
from os import path, chdir, getcwd, listdir

# query() is the bread and butter of interacting with an element in the database. Returns -1 if object's not found.
# TODO: You want to be able to navigate from your resources folder to your database directories. 
def query(python_obj):
  # Look for the appropriate directory of the attribute you want to search for
  found_db = False
  for _ in range(5):  # try 5 parent directories
    if "media.bin" in os.listdir("."):
      chdir("./db/%s/")
      break
  if not found_db:
    print("Didn't find database after traversing 5 parent directories to %s."%(getcwd()))
    quit()
  obj_dir = "%s%s%s"%(python_obj.type, SEP, python_obj.type)
  obj_db  = "../games/%s/db_%s"%(game_name, python_obj.type)
  if attr_name is None:
    filename = "%s%s%s_db.c"%(obj_dir, SEP, python_obj.name)
  else:
    filename = "%s%s%s%sattr%s%s_db.c"%(obj_dir, SEP, python_obj.name, SEP, SEP, attr_name)
    
  # Insert/update overarching table
  f = open(filename, "r");
  # Trusting that Python's class data aligns with C's struct data...
  file_lines = f.readlines();
  f.close();
  # See if element we're merging exists yet in file
  for line_idx in range(len(file_lines)):
    if python_obj.name in file_lines[line_idx].split():
      return line_idx;
  return -1;

# mod() stands for Merge Or Delete (play on words-- MODify, hee hee heeeeeeeee k bad joke)
def mod(mod_type, python_obj, attr_name=None, attr_val=None):
  obj_dir = "%s%s%s"%(python_obj.type, SEP, python_obj.type)
  obj_db  = "../games/%s/db_%s"%(game_name, python_obj.type)
  if attr_name is None:
    filename = "%s%s%s_db.c"%(obj_dir, SEP, python_obj.name)
  else:
    filename = "%s%s%s%sattr%s%s_db.c"%(obj_dir, SEP, python_obj.name, SEP, SEP, attr_name)
    
  # Insert/update overarching table
  # See if element we're merging exists yet in file
  for line_idx in range(len(file_lines)):
    if python_obj.name in file_lines(line_idx):
      before_obj_lines = file_lines[:line_idx]
      if (line_idx != len(file_lines) - 1):
        after_obj_lines  = file_lines[(line_idx + 1):]
      else:
        after_obj_lines = "\n}"
      obj_exists_in_db = True
    break
  # If element is found in DB, then copy the lines before and after. Then paste the before-lines, the new lines, and after.
  if obj_exists_in_db:
    # If any of the object's attributes are structs, they must go into another DB in current directory's child folder.
    for obj_attr_name in python_obj.__dict__.keys():
      # If this object is a Python class (struct), recurse for the next directory.
      if type(obj_attr_name) == "instance":
        mod(mod_type, python_obj, attr_name, eval("%s.%s")%(python_obj, attr_name))  # eval(...) gets the value of the attribute from this object
      else:
        f = open(filename, "w")
        f.write(before_obj_lines)
        if mod_type == MERGE:
          f.write("\t%s,\n"%(python_obj.__dict__.keysobj_attr_name]))
        f.write(after_obj_lines)
        f.close()


# merge() both inserts and updates, since we're breaking everything up into separate files.
def merge(python_obj, game_name, attr_name=None):
  mod(MERGE, python_obj, game_name, attr_name)

# delete() just removes a row from a table and all of the 
def delete(python_obj, game_name, attr_name=None):
  mod(DELETE, python_obj, game_name, attr_name)

