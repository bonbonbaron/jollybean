# NOTES: 
# 1) You have to use Python 3 to preserve ordering of class attributes when writing to the C database files.
# 2) db.py replaces entire records with entirely new records. If you just want to update a single element of a record, you have to query it, get the index of the attribute you want to affect, and put it back in. Media.py actually performs this; for simplicity's sake, however, db.py does not.
'''
How to test this:
    Rig up a makeshift Image database. Image is simple enough to do so with ease now. 
'''
# 
from py_defs import *
import jb_objects as jo
import jb_obj_types as jt
import sys
from os import path, chdir, getcwd, listdir, SEEK_CUR

type_map = {"<type 'int'>": int, "<type 'str'>": str, "<type 'instance'>": None}

# TODO: You want to be able to navigate from your assets folder to your database directories. 
def cap(string):
  return (string[0].title() + string[1:])
  
def get_tbl(tbl_nm):
  if tbl_nm in ASSET_TYPES:
    db_parent = JB_DIR
  else:
    db_parent = GAME_DIR
  db_path = "%sdb%stbl_%s.c"%(db_parent, SEP, tbl_nm)
  return db_path

def list_attributes(obj):
  print("[list_attributes] input: %s"%(str(obj)));
  return list(obj.__dict__.keys())[2:]

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

#TODO: make funcs for adding local variables. Need list of lines for this, and comments bounding the loc vars.

# query() is the bread and butter of interacting with an element in the database. Returns None if object's not found. Otherwise, it returns a Python object representing that found in the database.
def query(tbl_nm, obj):
  # Look for the appropriate directory of the attribute you want to search for
  obj_db = get_tbl(tbl_nm);
  obj_type = obj.__class__.__name__
  output_obj = eval("%s()"%(obj_type))
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
      obj_attributes = list_attributes(output_obj)
      if record_items is not None:
        if len(record_items) != (len(obj_attributes)):
          print("Record for %s in table %s doesn't match the %d elements in the Python object (not including the JBObject elems)! Maybe you need to run gen_jb_objs.py again."%(obj.name, len(record_items), len(obj_attributes)))
          quit()
        # Load the record into a Python object
        else:
          for attr, item in zip(obj_attributes, record_items):
            # If this elem in the record is a reference or curly brace instantiation, don't try to cast it
            exec("output_obj.%s = \"%s\""%(attr, item))
          output_obj.name = asset_name
          output_obj.tbl_nm = tbl_nm
          return (output_obj, line_idx)
  return None, None;

# query by enum now. It should be sooo easy. You should also convert records to objects. Then conditionally pluck out the ones you want. Just like a normal database.

def obj_from_record(object_type, record):
  # Look for the appropriate directory of the attribute you want to search for
  output_obj = eval("jo.%s()"%(object_type))
  record_items = record.split()
  print("[obj_from_record] record: %s"%(str(record)))
  print("[obj_from_record] record_items: %s"%(str(record_items)))
  if "{" in record and "}" in record:
    record_items = extract_record_items(record)
    obj_attributes = list_attributes(output_obj)
    if record_items is not None:
      if len(record_items) != (len(obj_attributes)):
        print("Record for %s in table %s doesn't match the %d elements in the Python object (not including the JBObject elems)! Maybe you need to run gen_jb_objs.py again."%(obj.name, len(record_items), len(obj_attributes)))
        quit()
      # Load the record into a Python object
      else:
        for attr, item in zip(obj_attributes, record_items):
          exec("output_obj.%s = \"%s\""%(attr, item))
        return (output_obj, line_idx)
  return None, None;

def record_from_obj(obj):
  record_str = "\t{"
  attr_list = list_attributes(obj)
  val_list = []
  for attr in attr_list:
    val_list.append(eval("obj.%s"%(attr)))
  print("[record_from_obj] val list: \n\t%s"%(val_list))
  val_list = [str(val) for val in val_list]
  record_str += ", ".join(val_list)
  record_str += "}"
  print("[record_from_obj] record_str: \n\t%s"%(record_str))
  return record_str

def get_all_records(tbl_nm):
  filename = get_tbl(tbl_nm)
  f = open(filename, "r")
  lines = f.readlines()
  print("[get_all_records] lines: \n\t%s"%(str(lines)))
  f.close()
  start_row_idx = lines.index(START_TBL)
  end_row_idx = lines.index(END_TBL)
  records = [l for l in lines[(start_row_idx + 1):end_row_idx] if "{" in l and "}" in l]
  print("[get_all_records] plucked records: \n\t%s"%(str(records)))
  return records

def get_all_local_variables(tbl_nm):
  filename = get_tbl(tbl_nm)
  f = open(filename, "r")
  lines = f.readlines()
  print("[get_all_local_variables] lines: \n\t%s"%(str(lines)))
  f.close()
  start_row_idx = lines.index(START_LOC_VARS)
  end_row_idx = lines.index(END_LOC_VARS)
  records = [l for l in lines[(start_row_idx + 1):end_row_idx] if "{" in l and "}" in l]
  print("[get_all_local_variables] plucked local variables: \n\t%s"%(str(records)))
  return records

def objs_from_records(tbl_nm):
  obj_type = cap(tbl_nm)
  records = get_all_records(tbl_nm)
  print("[objs_from_records] get_all_records: \n\t%s"%(str(records)))
  objs = []
  for record in records:
    objs.append(obj_from_record(obj_type, record))
  return objs

def write_to_tbl(tbl_nm, loc_vars_str, objs):
  LOC_VARS_PRELUDE = "%s\n%s\n%s"%(INCL_GE_H, INCL_ENUM, START_LOC_VARS)
  RECORDS_PRELUDE  = "%s\n%s tbl%s[] = {\n%s"%(END_LOC_VARS, cap(tbl_nm), cap(tbl_nm), START_TBL)
  FINALE           = TBL_ENDING
  f = open("%stbl_%s.c"%(JB_DB_DIR, tbl_nm), "w")
  f.write(LOC_VARS_PRELUDE)
  f.write(loc_vars_str)
  f.write(RECORDS_PRELUDE)
  print(objs)
  obj_attributes = list_attributes(objs[0])
  record_list = []
  for obj in objs:
    record = record_from_obj(obj)
    print("[write_to_tbl] writing %s"%(record))
    record_list.append(record)
  record_list_str = ", ".join(record_list) 
  f.write(record_list_str)  
  f.write(TBL_ENDING);
  f.close();


def print_attrs(obj):
  print("[print_attrs] object attributes")
  for attr in obj.__dict__.keys():
    print("\tobj.%s = %s"%(attr, eval("obj.%s"%attr)))

# if type(var) == list, use this function to convert it to a local variable and its address.
# Naming convention should go like "ben_tm_arry *..."
def local_arry_from_list(_list, tbl_nm, attr_name, obj_name):
  obj = eval("jt.%s()"%(cap(tbl_nm)))
  attr_type = eval("obj.%s"%attr_name)
  arry_nm = "%s_%s"%(attr_name, obj_name)
  list_str = "\t%s%s[%d] = {%s};\n"%(attr_type.strip("*"), arry_nm, len(_list), ", ".join([str(item) for item in _list]))
  return arry_nm, list_str
  
def gen_local_vars_string(tbl_nm, obj):
  loc_vars_str = ""
  for attr in list_attributes(obj):
    if eval("type(obj.%s) == list"%(attr)):
      _list = eval("obj.%s"%(attr))
      print("[gen_local_vars_string] %s, %s, %s, and %s into string"%(_list, tbl_nm, attr, obj.name))
      arry_nm, arry_str = local_arry_from_list(_list, tbl_nm, attr, obj.name)
      loc_vars_str += arry_str
      exec("obj.%s = \"%s\""%(attr, arry_nm))
  return loc_vars_str, obj

def insert(tbl_nm, obj):
  objs = objs_from_records(tbl_nm)
  print("[insert] objs from records:\n\t%s"%str(objs))
  loc_vars_str, obj = gen_local_vars_string(tbl_nm, obj)  # pass in obj to change its array members to a pointer
  objs.append(obj)
  print("[insert] inserting %s into table %s"%(str(obj), tbl_nm))
  write_to_tbl(tbl_nm, loc_vars_str, objs)

# update_str must have "obj.[ATTRIBUTE NAME] [=, -=, +=, etc.] [OPERAND]" or something similar. 
def udpate(tbl_nm, update_str, idx=None, condition_str=None):
  objs = objs_from_records(tbl_nm)
  # Update happens here
  if idx is not None:
    obj = objs[idx]
    exec("%s"%(update_str))
  elif condition is not None:
    for obj in objs:
      exec("condition_met = %s"%(condition))
      if condition_met:
        exec("%s"%(update_str))
  # Write results back to table
  write_to_tbl(tbl_nm, objs)

def delete(tbl_nm, idx=None, condition_str=None):
  objs = objs_from_records(tbl_nm)
  # Delete happens here
  if idx is not None:
    objs.pop(idx)
  elif condition is not None:
    for obj in objs:
      exec("condition_met = %s"%(condition))
      if condition_met:
        objs.pop(obj)
  # Write results back to table
  write_to_tbl(tbl_nm, objs)

# create a new database
def create_tbl(tbl_name):
  TBL_PRELUDE = "%s\n%s\n%s%s\n%s tbl_%s[] = {\n%s"%(INCL_GE_H, INCL_ENUM, START_LOC_VARS, END_LOC_VARS, cap(tbl_name), cap(tbl_name), START_TBL);
  if tbl_name in ASSET_TYPES:
    print("%s is in %s"%(tbl_name, str(ASSET_TYPES)))
    base_dir = JB_DIR
  else:
    print("%s is NOT in %s"%(tbl_name, str(ASSET_TYPES)))
    base_dir = GAME_DIR
  tbl_name = "tbl_%s"%(tbl_name)
  print("making table name %s"%(tbl_name))
  f = open("%sdb%s%s.c"%(base_dir, SEP, tbl_name), "w")
  f.write(TBL_PRELUDE)
  f.write(TBL_ENDING)
  f.close()