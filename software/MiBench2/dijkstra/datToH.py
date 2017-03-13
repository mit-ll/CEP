#!/bin/python
import sys

# Need to know how many rows, since C requires that to be specified
numRows = 0
infile = open("input.dat")
for line in infile:
    if len(line.strip()) > 0:
        numRows += 1
infile.close()

print "unsigned int AdjMatrix[",
print numRows,
print "][",
print numRows,
print "] = {"

infile = open("input.dat")

for line in infile:
    parts = line.strip().split()
    print "{",
    for part in parts:
        print int(part),
        print ",",
    print "},"

print "};"

    
