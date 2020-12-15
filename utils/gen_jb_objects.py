from py_defs import *
import CppHeaderParser as p

h = p.CppHeader("/home/bonbonbaron/hack/jollybean/src/jb.h")

f = open("jb_objects.py", "w")
f.write("from py_defs import JBObject\n\n")
for c in h.classes:
    f.write("class %s(JBObject):\n\tdef __init__(self):\n"%(c))
    f.write("\t\tJBObject.__init__(self)\n")
    for prop in h.classes[c]["properties"]["public"]:
        val = 0
        if prop["pointer"]:
            val = "None"
        f.write("\t\tself.%s = %s\n"%(prop["name"], val))
    f.write("\n")
