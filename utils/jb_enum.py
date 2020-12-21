from py_defs import *

ENUM_NAME_PREFIX = "e"
def create_enum_file():
  f = open(ENUM_FP, "w")
  f.close()

def read_enum_lines():
  f = open(ENUM_FP, "r")
  lines = f.readlines()
  return lines

def find_enum_idx(enum_nm):
  lines = read_enum_lines()
  for line in lines:
    words = line.split()
    if len(words) > 2:
      if words[0] == "enum" and words[1] == enum_nm:
        return lines.index(line)
  return -1

def get_enum(enum_nm):
  enum_idx = find_enum_idx(enum_nm)
  lines = read_enum_lines()
  if enum_idx < 0:
    return None
  return lines[enum_idx]

def get_enum_elems(enum_nm):
  e = get_enum(enum_nm)
  if e is not None:
    start = e.index("{") + 1
    end   = e.index("}")
    elem_string = e[start : end]
    elems = [i.strip(",") for i in elem_string.split()]
    print("Elements of enum %s: %s"%(enum_nm, elems))
    return elems
  return None

def query_enum_elem(elems, elem_nm):
  if elems is not None:
    return (elem_nm in elems)
  return None  # "None" tells us the enum doesn't exist to begin with. Distinct from "False", meaning you found the enum but the name wasn't in it.

def elems_to_enum(enum_nm, elems):
  list_string = ", ".join(elems)
  enum_string = "enum %s {%s};"%(enum_nm, list_string)
  return enum_string
  
def get_proper_enum_nm(nm):
  return "e%s"%(nm)

def get_proper_elem_nm(enum_nm, elem_nm):
  return "i%s_%s"%(enum_nm, elem_nm)

def update_enum_file(enum_nm, new_enum):
  idx = find_enum_idx(enum_nm)
  if idx > 0:
    f = open(ENUM_FP, "r")
    lines = f.readlines()
    f.close()
    f = open(ENUM_FP, "w")
    f.writelines(lines[:idx])
    f.write(new_enum)
    f.writelines(lines[(idx + 1):])
    f.write("\n")
    f.close()
  else:
    print("couldn't find enum %s! quitting..."%(enum_nm))
    quit()

def insert_enum_elem(enum_nm, elem_nm, idx=-1):   # element does not start with "e_" prefix
  proper_enum_nm = get_proper_enum_nm(enum_nm)
  elems = get_enum_elems(proper_enum_nm)
  if elems is not None:
    proper_elem_nm = get_proper_elem_nm(proper_enum_nm, elem_nm)
    elem_exists = query_enum_elem(elems, elem_nm)
    if elem_exists:
      print("[insert_enum_elem] %s already exists in enum %s! Quitting..."%(elem_nm, proper_enum_nm))
      quit()
    else:
      # Insert into enum
      if idx > 0:
        elems.insert(idx, proper_elem_nm)
      else:
        elems.append(proper_elem_nm)
      elems = list(set(elems))  # make sure there are no duplicates!
      print(elems)
      enum_string = elems_to_enum(proper_enum_nm, elems)
      update_enum_file(proper_enum_nm, enum_string)
  else:
    print("couldn't find enum %s that you wanted to insert into. Quitting..."%(enum_nm))

def create_enum(enum_nm):
  proper_enum_nm = get_proper_enum_nm(enum_nm)
  existing_enum = get_enum(proper_enum_nm)
  if (existing_enum is None):
    enum_string = "\nenum %s {};\n"%(proper_enum_nm)
    f = open(ENUM_FP, "a")
    f.write(enum_string)
    f.close()
  else:
    print("enum %s already exists! quitting..."%(enum_nm))

def delete_enum(enum_nm):
  proper_enum_nm = get_proper_enum_nm(enum_nm)
  idx = find_enum_idx(proper_enum_nm)
  if (idx >= 0):
    f = open(ENUM_FP, "r")
    lines = f.readlines()
    f.close()
    f = open(ENUM_FP, "w")
    f.writelines(lines[:idx])
    f.writelines(lines[(idx + 1):])
    f.close()
  else:
    print("Can't delete enum %s if it doesn't exist! Quitting..."%(enum_nm))
    quit()

def delete_enum_elem(enum_nm, elem_nm):
  proper_enum_nm = get_proper_enum_nm(enum_nm)
  elems = get_enum_elems(proper_enum_nm)
  proper_elem_nm = get_proper_elem_nm(proper_enum_nm, elem_nm)
  elems.pop(elems.index(proper_elem_nm))
  enum_string = elems_to_enum(proper_enum_nm, elems)
  update_enum_file(proper_enum_nm, enum_string)




