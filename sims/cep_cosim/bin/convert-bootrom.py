#!/usr/bin/python3
#//************************************************************************
#// Copyright 2022 Massachusets Institute of Technology
#//
#// File Name:      convert-bootrom.py
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Generates an ARM Compiled ROM compatible version
#//                 of the CEP Bootrom
#// Notes:          - Convert .img to .hex (as binary2ascii does)
#//                 - Pad out to a specified number of lines
#//
#//************************************************************************

import sys
import re
import os
import binascii

if (len(sys.argv) != 4):
  sys.exit(sys.argv[0] + " : [ERROR] Usage: <inputFile> <outputFile> <# of lines to pad to>")

# Save the arguments
inputFile    = sys.argv[1]
outputFile   = sys.argv[2]
padToLines   = sys.argv[3]

# DO some error checking
if (not os.path.exists(inputFile)) or (inputFile == outputFile):
  sys.exit(sys.argv[0] + " : [ERROR] inputFile does not exist or inputFile == outputFile.")

# Open input nad output files
b 		= open(inputFile, "rb").read()
nrows 	= 0

with open(outputFile, "w") as outfile:
    while b:
        row_bin = b[:8][::-1]
        b = b[8:]
        outfile.write("".join([bin(c)[2:].rjust(8,"0") for c in row_bin]).ljust(64,"0")+"\n")
        nrows += 1

    while nrows < int(padToLines):
        outfile.write("0"*64 + "\n")
        nrows += 1

