from py_defs import *
import os

def make_file(fp):
    f = open(fp, "w")
    f.close()

os.chdir(DEV_DIR)
make_file("./media.bin")
os.mkdir("jb")
os.mkdir("game")
os.mkdir("jb/db")
os.mkdir("game/db")
os.mkdir("assets")

