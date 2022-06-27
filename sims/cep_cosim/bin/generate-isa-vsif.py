#!/usr/bin/python3
#//************************************************************************
#// Copyright 2022 Massachusets Institute of Technology
#//
#// File Name:      generate-isa-vsif.oy
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Generates a VManager .vsif file for the RISC-V ISA Tests
#// Notes:
#//************************************************************************

import sys
import re
import os
import subprocess

if (len(sys.argv) != 3):
  sys.exit(sys.argv[0] + " : [ERROR] Usage: <isa directory> <destFile>")

# Save the arguments
inputDirectory	= sys.argv[1]
outputFile   	= sys.argv[2]

# DO some error checking
if (not os.path.exists(inputDirectory)):
  sys.exit(sys.argv[0] + " : [ERROR] specified isa directory does not exist")

print("--- " + sys.argv[0] + " ---")
print("ISA Source directory = " + sys.argv[1])
print("Output VSIF file     = " + sys.argv[2])

# Capture all tests for which a dump file has been generated
output 		= subprocess.getoutput("ls -1 " + sys.argv[1] +"rv64*dump | xargs -n 1 basename | sed -e 's/\\.dump$//'")
test_list	= output.split()

try: 
	f = open(outputFile, "w")
	f.write("session isaTests {\n")
	f.write("  top_dir : $ENV(MY_REGRESSION_AREA);\n")
	f.write("  output_mode : log_only;\n")
	f.write("  max_runs_in_parallel : 1;\n")
	f.write("  pre_session_script   : \"cd $ENV(CEP_INSTALL)/testSuites/isaTests; make cleanAll; make buildLibs\";\n")
	f.write("}\n")
	f.write("\n")
	f.write("group isaTests {\n")
	f.write("  scan_script: \"vm_scan.pl ius_hdl.flt\";\n")
	f.write("  timeout: 1500;\n")
	f.write("  drm: \"serial local\";\n")
	f.write("\n")

	for i in test_list:
		f.write("test " + i + "{\n")
		f.write("  run_script : \"cd $ENV(CEP_INSTALL)/testSuites/isaTests/" + i + "; make CADENCE=1 COVERAGE=1\";\n")
		f.write("}\n")
		f.write("\n")

	f.write("}\n")

finally:
	f.close()
	print("Operation complete\n")
