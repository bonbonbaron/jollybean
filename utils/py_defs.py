import os

INCL_GE_H = "#include \"ge.h\"\n"
START_ROW = "/* START_ROW_DEFS */\n"
END_ROW = "/* END_ROW_DEFS */\n"
TBL_DESC = "/* Idx #, Position array, Orientation array */\n"
START_TBL = "/* START_TBL_DEF */\n"
END_TBL = "/* END_TBL_DEF */\n"
SCENE = "scene"
SPRITE = "sprite"
AUDIO  = "audio"
TEXT   = "text"
IMAGE  = "image"

DBS = [SCENE, SPRITE, AUDIO, TEXT, IMAGE]
NULL = "NULL"
IS_LINUX = False
IS_WINDOWS = False
SEP = ""

# CRUD CONSTANTS
MERGE  = 1
DELETE = 2

if "/" in os.getcwd():
    SEP = "/"
    IS_LINUX = True
elif "\\" in os.getcwd():
    IS_WINDOWS = True
    SEP = "\\"
else:
    IS_LINUX = True
    SEP = "/"
