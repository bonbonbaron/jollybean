import os

if "/" in os.getcwd():
    SEP = "/"
    IS_LINUX = True
elif "\\" in os.getcwd():
    IS_WINDOWS = True
    SEP = "\\"
else:
    IS_LINUX = True
    SEP = "/"

INCL_GE_H = "#include \"jb.h\""
START_TBL = "/* START_TBL_RECORDS */\n"
END_TBL = "/* END_TBL_RECORDS */\n"
TBL_ENDING  = "\n%s};\n"%(END_TBL)
SCENE = "scene"
SPRITE = "sprite"
AUDIO  = "audio"
TEXT   = "text"
IMAGE  = "image"
TILESET = "tileset"
TILEMAP = "tilemap"
MEDIAINFO = "mediaInfo"
COLORPALETTE = "colorPalette"

DEV_DIR = "/home/bonbonbaron/nectar/"
ENUM_FP = "%sjb_enum.h"%(DEV_DIR)
JB_DIR  = "%sjb%s"%(DEV_DIR, SEP)
JB_DB_DIR = "%sdb%s"%(JB_DIR, SEP)
GAME_DIR = "%sgame%s"%(DEV_DIR, SEP)
ASSETS_DIR = "%sassets%s"%(DEV_DIR, SEP)
MEDIA_FP = "%smedia.bin"%(DEV_DIR)
ASSET_TYPES = [SCENE, SPRITE, AUDIO, TEXT, IMAGE, TILESET, TILEMAP, MEDIAINFO, COLORPALETTE]
ASSET_FILETYPES = ["?", "?", "mp3", "txt", "png"]
NULL = "NULL"

# CRUD CONSTANTS
MERGE  = 1
DELETE = 2

class JBObject:
    def __init__(self):
        self.name = ""
        self.tbl_name = ""
