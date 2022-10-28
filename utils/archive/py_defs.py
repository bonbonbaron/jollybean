import os

GAME_NAME = "nectar"

if "/" in os.getcwd():
    SEP = "/"
    IS_LINUX = True
elif "\\" in os.getcwd():
    IS_WINDOWS = True
    SEP = "\\"
else:
    IS_LINUX = True
    SEP = "/"

ROOT_DIR = "/home/bonbonbaron/games/"
GAME_DIR = ROOT_DIR + GAME_NAME + SEP
SRC_DATA_DIR = GAME_DIR + "srcData" + SEP
DST_DATA_DIR = GAME_DIR + "gameData" + SEP
