#!/usr/bin/python3
#//************************************************************************
#// Copyright 2022 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      sort-blackbox.py
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Sorts the firrt_black_box_resources_files.f file
#//                 based on prioritized package list
#// Notes:          
#//
#//************************************************************************

# replaces a `include with the full include file
#
# args
# $1 - File to sort
# $2 - File with list of prioritized packages
import sys
import re
import os

# Using list comprehensive to remove elements from a list that match a string
def check_pres(ele, test_str):
  if test_str in ele:
    return 0
  return 1

if (len(sys.argv) != 3):
  sys.exit("[ERROR] Only two arguments supported")

# Save the arguments
fileToSort    = sys.argv[1]
sortFile      = sys.argv[2]

# DO some error checking
if (not os.path.exists(fileToSort)) or (not os.path.exists(sortFile)) or (fileToSort == sortFile):
  sys.exit("[ERROR] Specified files don't exist or they are the same.")

# Read input file into a list (we will be writing back to the file)
fileToSort_fd = open(fileToSort, "r+")
fileToSort_list = fileToSort_fd.readlines()

# Read priority file
sortFile_fd = open(sortFile, "r")
sortFile_list = sortFile_fd.readlines()

# Remove any comments from the sortFile
#sortFile_list = [ele for ele in sortFile_list if check_pres(ele, '//')]
sortFile_list = [ele for ele in sortFile_list if check_pres(ele, '//')]

# Initialize the new content list
sorted_list = []

# Begin to build the sorted list
for x in sortFile_list:

  # Add the matching element to the sorted list
  sorted_list.append([ele for ele in fileToSort_list if x in ele])

  # Remove the matching element from the original list
  fileToSort_list = [ele for ele in fileToSort_list if check_pres(ele, x)]

# Concatenate the list
sorted_list = sorted_list + fileToSort_list

# Reset the file pointe and now write the new list to the original file
fileToSort_fd.seek(0)
for x in sorted_list:
  fileToSort_fd.writelines(x)

# Close file pointers
fileToSort_fd.close()
sortFile_fd.close()
