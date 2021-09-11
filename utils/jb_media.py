from py_defs import *
from jb_db import *

'''
  You gotta maintain media info in two different places:
    1) media.bin
    2) tbl_media

  This means that every CRUD operation on (1) is followed by the same on (2).

'''
def create_media_file():
  f = open(MEDIA_FP, "w")
  f.close()

def clean_media_file():
  create_media_file()

def query_media_tbl(name):
  py_obj = query("MediaInfo", name)
  if py_obj is None:
    print("Apparently jb_media.py can't find tblMediaInfo. Better check where it's looking for it.")
    quit()
  return py_obj

def delete(offset, length):
  f = open(MEDIA_FP, "rb")
  before_data = f.read(offset)
  #print("before: %s"%(before_data))
  f.seek(offset + length)
  after_data = f.read()
  #print("after: %s"%(after_data))
  f.close()
  f = open(MEDIA_FP, "wb")
  f.write(before_data)
  f.write(after_data)
  f.close()

def insert(data):
  f = open(MEDIA_FP, "ab")
  f.write(data)
  f.close()

# update() replacees a length-sized string of bytes with ANY-sized string of bytes: e.g. 12345 -> 1298745
def update(offset, length, new_data):
  f = open(MEDIA_FP, "rb")
  before_data = f.read(offset)
  f.seek(offset + length)
  after_data = f.read()
  f.close()
  f = open(MEDIA_FP, "wb")
  f.write(before_data)
  f.write(new_data)
  f.write(after_data)
  f.close()

# This is just for debugging purposes. 
def print_file():
  f = open(MEDIA_FP, "r")
  print(f.read())
  f.close()

def ensure_json_exists():
  json_fp = "%sjson_media.json"%(DEV_DIR)
  if not path.exists(json_fp):
    f = open(json_fp, "w")
    f.close()

def insert_record():
  ensure_json_exists()



def update_record():
  pass

def delete_record():
  pass

'''
We need a JSON file to keep the media enums, offsets, and lengths straight. JSON file will have an additional member, enum_name, that'll do this. JSON ties it all together.

CRUD operations flow like this:
  JSON > TBL > ENUM > MEDIA.BIN

JSON:
    Any time you CRUD, affect the JSON object as well.
    C:
        Append to media.json
        Append to tbl_media.c
        Append to jb_enum.h
        Append to media.bin
    U:
        Update media.json
            Remember to update all the affected offsets
        Rewrite all of tbl_media.c
        Don't touch jb_enum.h. Index stays the same in an update.
        Update media.bin
    D:
        Delete from media.json
            Remember to update all the affected offsets
        Rewrite all of tbl_media.c
        Delete enum element in jb_enum.h.
        Delete from media.bin

  
  However, I will be coming back to this problem once again when I make color palette, battle stat, etc. tables. So what I need to do is generalize the solution into separate jb_*.py documents:

  jb_json.py
  jb_db.py
  jb_enum.py
  jb_media.py
  jb_tbl.py

  Obviously, I've got a lot of these done already. Great! So now all I have to do is work on the ones I don't. Right now, I think that is just jb_db.py and jb_json.py.
  
  In the end, any time I create a new media file like a sprite animation strip, a sound effect, or a background song, I'm going to just call one function. This function will have a case structure. It's very important that the JSON function converts the 
'''

ensure_json_exists()
