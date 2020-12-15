# NOTE: you have to use Python 3 to preserve ordering of class attributes when writing to the C database files.
from py_defs import *
from jb_objects import *
import sys
from os import path, chdir, getcwd, listdir

type_map = {"<type 'int'>": int, "<type 'str'>": str, "<type 'instance'>": None}

# TODO: You want to be able to navigate from your assets folder to your database directories. 
def get_tbl(tbl_nm):
  if tbl_nm in ASSET_TYPES:
    db_parent = JB_DIR
  else:
    db_parent = GAME_DIR
  db_path = "%sdb%s%s%sdb_%s.h"%(db_parent, SEP, tbl_nm, SEP, tbl_nm)
  return db_path

def list_attributes(obj):
  return list(obj.__dict__.keys())
# query() is the bread and butter of interacting with an element in the database. Returns None if object's not found. Otherwise, it returns a Python object representing that found in the database.
def query(tbl_nm, asset_name):
  # Look for the appropriate directory of the attribute you want to search for
  obj_db = get_tbl(tbl_nm);
  jb_obj = eval("%s()"%(tbl_nm.lower()));
  # Open table in read-mode.
  f = open(obj_db, "r");
  file_lines = f.readlines();
  f.close();
  # See if element we're merging exists yet in file
  asset_name = asset_name.strip().lower()
  for line_idx in range(len(file_lines)):
    if asset_name == file_lines[line_idx].strip("/**/").strip().lower():
      # First, get the internals of the brackets and split by commas.
      line = file_lines[line_idx + 1]
      if "{" in line or "}" in line:
        line = [l.strip() for l in line.strip("{}").split(",")]
        obj_attributes = list_attributes(jb_obj)
        if len(line) != (len(obj_attributes) - 2):
          print("Record for %s in table %s doesn't match the %d elements in the Python object (not including the JBObject elems)! Maybe you need to run gen_jb_objs.py again.")
          quit()
        # Load the record into a Python object
        for attr, item in zip(obj_attributes, line):
          # If this elem in the record is a reference or curly brace instantiation, don't try to cast it
          eval("jb_obj.%s = %s"%(attr, item))
      jb_obj.name = asset_name
      jb_obj.tbl_name = tbl_nm
      return (jb_obj, line_idx)
  return None;


# mod() stands for Merge Or Delete
def mod(mod_type, tbl_nm, input_obj):
  # See if element we're merging exists yet in file
  db_obj, line_idx = query(input_obj.type, input_obj.name)
  # If element is found in DB, then copy the lines before and after. Then paste the before-lines, the new lines, and after.
  if db_obj is not None:
    # If any of the object's attributes are structs, they must go into another DB in current directory's child folder.
    in_obj_attrs = list_attributes(input_obj)
    f = open(filename, "r")
    all_lines = f.readlines()
    before_obj_lines = all_lines[:line_idx]
    after_obj_lines  = all_lines[(line_idx + 2):]
    f.close()
    # Merge or delete record here.
    f = open(filename, "w")
    f.write(before_obj_lines)
    if mod_type == MERGE:
      f.write("%s %s = {")
      # For simplicity, assume no instance attributes. Even those that are should simply be strings; e.g. "{1, 123, "abc", 12}". Even strings like "&img_boy" are fine.
      for attr in in_obj_attrs:
        f.write(str(eval("%s.%s"%(input_obj, attr))))
      f.write("};\n")
    f.write(after_obj_lines)
    f.close()
  # If it's not in the DB, then just append it to the end of the table. 
  else:
    f = open(filename, "a");
    f.write("%s %s = {"%(input_obj.tbl_nm, input_obj.name))
    attributes = list_attributes(input_obj);
    for idx in range(len(attributes)):
      f.write("%s", eval("input_obj.%s"%attributes[idx]))
      if idx < (len(attributes - 1)):
        f.write(", ")
    f.write("};\n")
    f.close()



# merge() both inserts and updates, since we're breaking everything up into separate files.
def merge(python_obj, attr_name=None):
  mod(MERGE, python_obj,  attr_name)


# delete() just removes a row from a table and all of the 
def delete(python_obj, attr_name=None):
  mod(DELETE, python_obj, attr_name)


# create a new database
def create_db(db_name):
  if db_name.lower().strip() in ASSET_TYPES:
    base_dir = JB_DIR
  else:
    base_dir = GAME_DIR
  f = open("%sdb%sdb_%s.h"%(base_dir, SEP, db_name), "w")
  f.write("%s\n%s%s%s%s%s"%(INCL_GE_H, START_ROW, END_ROW, TBL_DESC, START_TBL, END_TBL))
  f.close()
