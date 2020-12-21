import db
import media
import enum
import os
from py_defs import *
import glob

obj_type = os.getcwd().split(SEP)[-1]
filetype_mapping = {"sprite": "png", "image": "png", "bg": "png"}


asset_subdirs = os.listdir(ASSETS_DIR)
for asset_subdir in asset_subdirs:
  filetype = filetype_mapping.get(asset_subdir)
  print(asset_subdir)
  os.chdir("%s%s%s"%(ASSETS_DIR, asset_subdir, SEP))
  print("cur dir: %s"%(os.getcwd()))
  files = glob.glob("*.%s"%(filetype))
  print(files)
