#!/usr/bin/python3
#//************************************************************************
#// Copyright 2022 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      manage-submodules.py
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Supports exporting/importing of the CEP releases
#//                 from the internal to external repositories  
#// Notes:          export is run from the internal repository
#//                 import is run from the external repository
#//************************************************************************

# replaces a `include with the full include file
#
# args
# $1 - Operation: 'import' or 'export'
import os
import sys
import subprocess
import csv

# subModules to exclude from the export operation
# The two vlsi submodules have restricted access and cannot be cloned
excludeList = ["CEP_Chipyard_ASIC", "vlsi/hammer-mentor-plugins", "vlsi/hammer-synopsys-plugins"]
fileName    = "./gitSubmoduleExport.csv"

# Check the arguments
if (len(sys.argv) != 2 or (sys.argv[1] != "import" and sys.argv[1] != "export")):
  sys.exit("Usage: " + __file__ +" <import | export>")

# Grab the repo's root directory and change current working directory accordingly
repoRoot = subprocess.run(["git", "rev-parse", "--show-toplevel"], stdout=subprocess.PIPE, check=True)
os.chdir(repoRoot.stdout.strip())
repoRootDir = os.getcwd()

print("")
print("------------------------------------------------")
print("      CEP Submodule export/import script")
print("------------------------------------------------")
print("Current working directory: {0}".format(repoRootDir))

#//************************************************************************
#// Perform an export
#//************************************************************************
if (sys.argv[1] == "export"):

  # Some error checking
  if (not os.path.exists(".gitmodules")):
    sys.exit("[ERROR] : .gitmodules does not exist")
  
  # Run the git submodule command with the output being a list of lists, which each sublist being a pair of commit and path
  gitSubmoduleStatus = subprocess.run(["git", "submodule"], stdout=subprocess.PIPE, check=True).stdout
  gitSubmoduleStatus = gitSubmoduleStatus.decode('utf-8') if gitSubmoduleStatus else u''
  gitSubmoduleStatus = gitSubmoduleStatus.split()
  gitSubmoduleStatus = [x for x in gitSubmoduleStatus if not x.startswith('(')]
  gitSubmoduleStatus = [x.replace("-", "") if x.startswith('-') else x for x in gitSubmoduleStatus]
  gitSubmoduleStatus = [gitSubmoduleStatus[x:x+2] for x in range(0, len(gitSubmoduleStatus), 2)]


  # Read .gitmodules into a list and process it
  with open(".gitmodules") as file:
    gitModules = file.readlines()
  gitModules = [x.strip() for x in gitModules]
  gitModules = [x for x in gitModules if x.startswith('[submodule') or x.startswith('path') or x.startswith('url')]
  gitModules = [x.replace('[submodule "', '') for x in gitModules]
  gitModules = [x.replace('"]', '') for x in gitModules]
  gitModules = [x.replace('path = ', '') for x in gitModules]
  gitModules = [x.replace('url = ', '') for x in gitModules]
  gitModules = [gitModules[x:x+3] for x in range(0, len(gitModules), 3)]
  
  print("Submodule exclude list          : " + str(excludeList))

  # The lists should be of equal length
  if (len(gitSubmoduleStatus) != len(gitModules)):
    sys.exit("[ERROR] : length mismatch between gitSubmoduleStatus and gitModules")
  else:
    print("Total submodules                : " + str(len(gitSubmoduleStatus)))

  # gitSubmoduleStatus now has "doubles" of submodule commit & path
  # gitModules now has "triples" of submodule name, path, and url

  # Sort each list by the 2nd element (submodule path) of each entry
  gitSubmoduleStatus = sorted(gitSubmoduleStatus, key = lambda x: x[1])
  gitModules = sorted(gitModules, key = lambda x: x[1])
  
  # Define resulting list
  gitSubmoduleExport = []

  # Iterate through each list simultaneously
  for (submodule, module) in zip(gitSubmoduleStatus, gitModules):
    gitSubmoduleExport.append([module[0], module[1], module[2], submodule[0]])

  # Remove those submodules on the exclude list
  gitSubmoduleExport = [x for x in gitSubmoduleExport if not x[0] in excludeList]

  print("Total submodules post exclusion : " + str(len(gitSubmoduleExport)))

  # Write the results to the CSV file
  with open(fileName, "w") as f:
    wr = csv.writer(f)
    wr.writerows(gitSubmoduleExport)

  print("")
  print("Export complete.  Results writen to " + fileName)
  print("")

#//************************************************************************
#// Perform an import
#//************************************************************************
else:
  # Perform some error checking first
  if (not os.path.exists(fileName)):
    sys.exit("[ERROR] : Can't find " + fileName)

  # Read the CSV file into a list of lists
  with open(fileName, "r") as read_obj:
    csv_reader          = csv.reader(read_obj)
    gitSubmoduleImport  = list(csv_reader)

  # Iterate the list
  # submodule[0] is submodule name
  # submodule[1] is submodule path
  # submodule[2] is submodule url
  # submodule[3] is submodule commit
  for submodule in gitSubmoduleImport:
    
    try:
      print("Removing submodule at path: " + submodule[1])
      subprocess.run(["git", "rm", "-r", "--force", submodule[1]], stdout=subprocess.PIPE, check=True)
    except:
      print("Removal failed")

    print("Adding submodule " + submodule[0] + " from URL : " + submodule[2])
    
    # Remove the submodule path, if it exists
    if (os.path.exists(submodule[1])):
      subprocess.run(["rm", "-rf", submodule[1]], stdout=subprocess.PIPE, check=True)

    # Add the submodule and append to the .gitmodules file
    subprocess.run(["git", "submodule", "add", "--force", "--name", submodule[0], submodule[2], submodule[1]], stdout=subprocess.PIPE, check=True)

    # Checkout the specific commit for the current submodule
    os.chdir(submodule[1])
    print("Checking out commit " + submodule[3] + " ...")
    subprocess.run(["git", "checkout", "--quiet", submodule[3]], stdout=subprocess.PIPE, check=True)
    os.chdir(repoRootDir)

  # Even the excluded paths will have empty directories in the external repo.  Let's remove them.
  print("Removing exluded submodule directories...")
  for excludeDir in excludeList:
  	subprocess.run(["rm", "-rf", excludeDir], stdout=subprocess.PIPE, check=True)

  print("")
  print("Import complete.")
  print("")
