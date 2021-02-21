#!/usr/bin/python3

import sys
import os

filename = str(sys.argv[1])
print ('Parse ', filename)

f = open(filename, "r")
subs = f.read().split("?c") 

#print(subs)
basename = os.path.basename(filename) 
base = basename.split(".")[0]

file = open("./src/chtml/{}.c".format(base), "w")
file.write("#include \"request.h\"\n")
file.write("#include \"output.h\"\n\n")
file.write("void Output_")
file.write(base)
file.write("(STREAM stream) {\n")

mode = 0

for part in subs:
    if mode == 0:
        file.write("    OUT(\"")
        file.write(part.replace("\\", "\\\\").replace("\n", "\\n").replace("\"", "\\\"").replace("%", "%%"))
        file.write("\");\n")
        mode = 1
    else:
        file.write("    ")
        file.write(part.lstrip())
        file.write("\n")
        mode = 0

file.write("}\n")
file.close()

found = False
value = "extern void Output_{}(STREAM stream);".format(base)
with open('./inc/chtml/chtml_files.h') as header:
    if "Output_{}".format(base) in header.read():
        found = True
header.close()

if not found:
    header = open('./inc/chtml/chtml_files.h','a+')
    header.write(value+"\n")
    header.close()


