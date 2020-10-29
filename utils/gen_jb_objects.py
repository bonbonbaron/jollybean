from py_defs import *
import CppHeaderParser as p

h = p.CppHeader("..%sjb.h"%SEP)

f = open("jb_objects.py", "w")

for c in h.classes:
    f.write("class %s:\n\tdef __init__(self):\n"%(c))
    for prop in h.classes[c]["properties"]["public"]:
        val = 0
        if prop["pointer"]:
            val = "None"
        f.write("\t\tself.%s = %s\n"%(prop["name"], val))
    f.write("\n")
