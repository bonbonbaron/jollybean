# NOTES: 
# 1) You have to use Python 3 to preserve ordering of class attributes when writing to the C database files.
# 2) db.py replaces entire records with entirely new records. If you just want to update a single element of a record, you have to query it, get the index of the attribute you want to affect, and put it back in. Media.py actually performs this; for simplicity's sake, however, db.py does not.
'''
How to test this:
    Rig up a makeshift Image database. Image is simple enough to do so with ease now. 
'''
# 
from py_defs import *
from jb_objects import *
import sys
from os import path, chdir, getcwd, listdir, SEEK_CUR

type_map = {"<type 'int'>": int, "<type 'str'>": str, "<type 'instance'>": None}

# TODO: You want to be able to navigate from your assets folder to your database directories. 
def get_tbl(tbl_nm):
  if tbl_nm in ASSET_TYPES:
    db_parent = JB_DIR
  else:
    db_parent = GAME_DIR
  db_path = "%sdb%sdb_%s.h"%(db_parent, SEP, tbl_nm)
  return db_path

def list_attributes(obj):
  return list(obj.__dict__.keys())

def list_attr_vals(obj):
  if obj is not None:
    attrs = list_attributes(obj)
    for i in range(len(attrs)):
      vals.append(eval("obj.%s"%(attrs[i])))
    return vals
  return None

tmp = JBObject()
NUM_JB_ATTRS = len(list_attributes(tmp))

def extract_record_items(record):
  if "{" not in record:
    return None
  items = record.split()
  while 1:
    if "{" not in items[0]:
      items.pop(0)
    else:
      items[0] = items[0].strip("{")
      break;
  items[-1] = items[-1].strip("};\n")
  items = [i.strip(", ") for i in items]
  return items

# query() is the bread and butter of interacting with an element in the database. Returns None if object's not found. Otherwise, it returns a Python object representing that found in the database.
def query(tbl_nm, obj):
  # Look for the appropriate directory of the attribute you want to search for
  obj_db = get_tbl(tbl_nm);
  obj_type = obj.__class__.__name__
  output_obj = eval("%s()"%(obj_type))
  jb_obj = eval("%s()"%(obj_type));
  # Open table in read-mode.
  f = open(obj_db, "r");
  file_lines = f.readlines();
  f.close();
  # See if element we're merging exists yet in file
  asset_name = obj.name.strip().lower()
  try:
    start_idx = file_lines.index(START_TBL)
  except:
    start_idx = 0

  for line_idx in range(start_idx, len(file_lines)):
    line = file_lines[line_idx]
    items = line.split()
    if len(items) > 2 and items [1] == obj.name and "{" in line and "}" in line:
      record_items = extract_record_items(line)
      obj_attributes = list_attributes(jb_obj)[NUM_JB_ATTRS:]
      if record_items is not None:
        if len(record_items) != (len(obj_attributes)):
          print("Record for %s in table %s doesn't match the %d elements in the Python object (not including the JBObject elems)! Maybe you need to run gen_jb_objs.py again."%(obj.name, len(record_items), len(obj_attributes)))
          quit()
        # Load the record into a Python object
        else:
          for attr, item in zip(obj_attributes, record_items):
            # If this elem in the record is a reference or curly brace instantiation, don't try to cast it
            exec("output_obj.%s = \"%s\""%(attr, item))
          jb_obj.name = asset_name
          jb_obj.tbl_name = tbl_nm
          return (jb_obj, line_idx)
  return None, None;


# mod() stands for Merge Or Delete
def mod(mod_type, tbl_nm, input_obj):
  # See if element we're merging exists yet in file
  found_db_obj, found_obj_idx = query(input_obj.tbl_name, input_obj)
  # If element is found in DB, then copy the lines before and after. Then paste the before-lines, the new lines, and after.
  filename = get_tbl(tbl_nm)
  f = open(filename, "r")
  lines = f.readlines()
  f.close()
  #print("\n\n~~~~~~~~~~~~\nDBNOW:\n~~~~~~~~~~\n")
  #f = open(filename, "r")
  #txt = f.read()
  #f.close()
  #print(txt)
  #print("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n")
  start_row_idx = lines.index(START_TBL)
  end_row_idx = lines.index(END_TBL)
  f = open(filename, "w")
  # write everything up until /* START_TBL_DEFS */
  first_record_line_idx = start_row_idx + 1
  f.writelines(lines[:first_record_line_idx])
  if found_db_obj is not None:
    #######################
    ### EXISTING RECORD ###
    #######################
    in_obj_attrs = list_attributes(input_obj)
    #######################
    ### MERGE OR DELETE ###
    #######################
    if mod_type == MERGE:
      before_obj_lines = lines[first_record_line_idx : found_obj_idx]
      #print("\n\nBEFORE OBJECT LINES:")
      #print(before_obj_lines)
      f.writelines(before_obj_lines)
      f.write("\t/* %s */ {"%(input_obj.name))
      # For simplicity, assume no instance attributes. Even those that are should simply be strings; e.g. "{1, 123, "abc", 12}". Even strings like "&img_boy" are fine.
      for i in range(NUM_JB_ATTRS, len(in_obj_attrs)):
        f.write(str(eval("input_obj.%s"%(in_obj_attrs[i]))))
        if (i != (len(in_obj_attrs) - 1)):
          f.write(", ")
      f.write("}, \n")
      after_obj_lines  = lines[(found_obj_idx + 1):]
    elif mod_type == DELETE:
      before_obj_lines = lines[first_record_line_idx : (found_obj_idx - 1)]
      #print("\n\nBEFORE OBJECT LINES:")
      #print(before_obj_lines)
      f.writelines(before_obj_lines)
      if found_obj_idx == (end_row_idx - 1):
        last_record = lines[(found_obj_idx - 1)]
        last_record = "%s\n"%(last_record[:-2])
        f.write(last_record)
      else:
        f.write(lines[found_obj_idx - 1])
      after_obj_lines  = lines[(found_obj_idx + 1):]
  # If it's not in the DB, then just append it to the end of the table. 
  else:
    ##################
    ### NEW RECORD ###
    ##################
    # Rewrite all records except the last one 
    # Then add a comma to the last record if a record exists
    f.writelines(lines[first_record_line_idx:(end_row_idx - 1)])  # excludes last record
    if ((end_row_idx - start_row_idx) > 1):
      last_record = lines[end_row_idx - 1]
      last_record = "%s,\n"%(last_record[:-1])
      f.write(last_record)
    f.write("\t/* %s */ {"%(input_obj.name))
    attributes = list_attributes(input_obj);
    ##############################
    ### NEW OBJECT ATTRIBUTES ####
    ##############################
    # Write all this obj's attributes EXCEPT for the JBObject attributes.
    for idx in range(NUM_JB_ATTRS, len(attributes)):
      f.write("%s"%(eval("input_obj.%s"%attributes[idx])))
      if idx < (len(attributes) - 1):
        f.write(", ")
    f.write("}\n")
    after_obj_lines  = lines[end_row_idx:]
  f.writelines(after_obj_lines)
  f.close()



# merge() both inserts and updates, since we're breaking everything up into separate files.
def merge(tbl_nm, input_obj):
  mod(MERGE, tbl_nm, input_obj)


# delete() just removes a row from a table and all of the 
def delete(tbl_nm, input_obj):
  mod(DELETE, tbl_nm, input_obj)


# create a new database
def create_db(db_name):
  if db_name.lower().strip() in ASSET_TYPES:
    base_dir = JB_DIR
  else:
    base_dir = GAME_DIR
  f = open("%sdb%sdb_%s.h"%(base_dir, SEP, db_name), "w")
  f.write("%s\n"%(INCL_GE_H))
  f.write("%s[] = {\n%s%s};\n"%(db_name.title(), START_TBL, END_TBL));
  f.close()
