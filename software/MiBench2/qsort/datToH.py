import sys

infile = open(sys.argv[1])

print "struct my3DVertexStruct {"
print "    int x, y, z;"
print "    double distance;"
print "};"
print
print "struct my3DVertexStruct array[] =  {\\"

for line in infile:
    parts = line.split()

    if len(parts) != 3:
        continue
    
    print "  {" + parts[0] + ", " + parts[1] + ", " + parts[2] + "}, \\"

print "};"
