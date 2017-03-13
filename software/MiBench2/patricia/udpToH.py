#!/bin/python

print "struct input_data_format {"
print "    float time;"
print "    char addr[26];"
print "};"
print

print "struct input_data_format input_data[] = {"

infile = open("small.udp")

for line in infile:
    parts = line.strip().split()
    addr =  '"' + parts[1] + " " + parts[2] + " " + parts[3] + " " + parts [4] + '"' + "},"
    print "{" + parts[0] + ", " + addr

print "};"

print
print "struct input_data_format * fakeFile;"
