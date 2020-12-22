from py_defs import *
import CppHeaderParser as p

h = p.CppHeader("/home/bonbonbaron/hack/jollybean/src/jb.h")

f1 = open("jb_objects.py", "w")
f2 = open("jb_obj_types.py", "w")
f1.write("from py_defs import JBObject\n\n")
f2.write("from py_defs import JBObject\n\n")
for c in h.classes:
    f1.write("class %s(JBObject):\n\tdef __init__(self):\n"%(c))
    f2.write("class %s(JBObject):\n\tdef __init__(self):\n"%(c))
    f1.write("\t\tJBObject.__init__(self)\n")
    f2.write("\t\tJBObject.__init__(self)\n")
    for prop in h.classes[c]["properties"]["public"]:
        val = 0
        if prop["pointer"]:
            val = "None"
        f1.write("\t\tself.%s = %s\n"%(prop["name"], val))
        f2.write("\t\tself.%s = \"%s\"\n"%(prop["name"], prop["type"]))
    f1.write("\n")
