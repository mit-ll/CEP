#!/bin/bash
#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      get_external_dependencies.sh
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Shell script to pull copies of external repositories
#//                 that the CEP depends on, but have not been directly
#//                 for a variety of reasons (including licensing)
#// Notes:          The "curl" commands have been locked into the particular
#//                 version of the repository that was tested with the CEP.
#//
#//************************************************************************

# The following decleration defines all the external CEP Dependencies that have not been directly included in
# the CEP repo.
#
# CAUTION: Listing directories within this construct MAY result in them being overwritten by the script
# 
# The pixman submodule within riscv-qemu has explicitly not been included at this time due to it being sourced
# on a seperate server.
#
# The format is:
#   GITHUB  - repo location, specific commit, target (relative) directory
#   GIT     - repo site, repo name, specific commit, target (relative) directory
#   HTTP    - file location, target (relative) directory
#
# The entries that have been commented out reflect dependencies that have been directly included within the CEP repo.  Unc               ommenting
# them and running this script will result in clobbering the current directory (and potentially loosing CEP-specific modifications)
#
# When a respository is commented out (such as https://github.com/sifive/freedom), this is included as a reference pointer to the original
#   repository commit that the current codebase was "forked" from...
#
declare -a repo_names=(       "#"
                              "# The following dependencies are related to the Freedom U500 Platform"
                              "#"
                              #"GITHUB https://github.com/sifive/freedom f8b917d ./hdl_cores/freedom"
                              #"GITHUB https://github.com/sifive/fpga-shells 0883312 ./hdl_cores/freedom/fpga-shells"
                              "GITHUB https://github.com/sifive/sifive-blocks 12bdbe5 ./hdl_cores/freedom/sifive-blocks"                             
                              "#"
                              "# The following dependencies are related to the Rocket Chip"
                              "#"
                              "GITHUB https://github.com/chipsalliance/rocket-chip d2210f9 ./hdl_cores/freedom/rocket-chip"
                              "GITHUB https://github.com/freechipsproject/chisel3 7a343dc ./hdl_cores/freedom/rocket-chip/chisel3"
                              "GITHUB https://github.com/freechipsproject/firrtl ad31c11 ./hdl_cores/freedom/rocket-chip/firrtl"
                              "GITHUB https://github.com/ucb-bar/berkeley-hardfloat 3f88b66 ./hdl_cores/freedom/rocket-chip/hardfloat"
                              "GITHUB https://github.com/ucb-bar/riscv-torture 77195ab ./hdl_cores/freedom/rocket-chip/torture"
                              "GITHUB https://github.com/chipsalliance/api-config-chipsalliance d619ca8 ./hdl_cores/freedom/rocket-chip/api-config-chipsalliance"
                              "#"
                              "# The following dependencies are related to OpenTitan"
                              "#"
                              "GITHUB https://github.com/lowRISC/opentitan 14b1b20 ./opentitan"
                              "#"
                              "# The following dependencies are related to the RISC-V GNU Toolchain"
                              "#"
                              "GITHUB https://github.com/riscv/riscv-gnu-toolchain 6d2706f ./software/riscv-gnu-toolchain"
                              "GITHUB https://github.com/riscv/riscv-binutils-gdb d7f734b ./software/riscv-gnu-toolchain/riscv-binutils"
                              "GITHUB https://github.com/riscv/riscv-dejagnu 4ea498a ./software/riscv-gnu-toolchain/riscv-dejagnu"
                              "GITHUB https://github.com/riscv/riscv-gcc 54945eb ./software/riscv-gnu-toolchain/riscv-gcc"
                              "GITHUB https://github.com/riscv/riscv-binutils-gdb c3eb407 ./software/riscv-gnu-toolchain/riscv-gdb"
                              "GITHUB https://github.com/riscv/riscv-glibc 06983fe ./software/riscv-gnu-toolchain/riscv-glibc"
                              "GITHUB https://github.com/riscv/riscv-newlib f289cef ./software/riscv-gnu-toolchain/riscv-newlib"                             
                              "#"
                              "# The following dependencies are related to the RISC-V Tests"
                              "#"
                              "GITHUB https://github.com/riscv/riscv-tests a5d8386 ./software/riscv-tests"              
                              "GITHUB https://github.com/riscv/riscv-test-env 4d4a435 ./software/riscv-tests/env"              
                              "#"
                              "# The following dependencies are related to the Freedom-U-SDK"
                              "#"
                              "GITHUB https://github.com/mcd500/freedom-u-sdk 29fe529 ./software/freedom-u-sdk"
                              "GITHUB https://github.com/sifive/buildroot c4ddfe7 ./software/freedom-u-sdk/buildroot"
                              "GITHUB https://github.com/riscv/riscv-linux 2751b6aa ./software/freedom-u-sdk/linux"
                              "GITHUB https://github.com/riscv/riscv-fesvr f683e01 ./software/freedom-u-sdk/riscv-fesvr"
                              "GITHUB https://github.com/riscv/riscv-gnu-toolchain b4dae89 ./software/freedom-u-sdk/riscv-gnu-toolchain"
                              "GITHUB https://github.com/riscv/riscv-binutils-gdb d0176cb ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-binutils-gdb"
                              "GITHUB https://github.com/riscv/riscv-dejagnu 5f3adaf ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-dejagnu"
                              "GITHUB https://github.com/riscv/riscv-gcc b731149 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-gcc"
                              "GITHUB https://github.com/riscv/riscv-glibc 2f626de ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-glibc"
                              "GITHUB https://github.com/riscv/riscv-newlib ccd8a0a ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-newlib"
                              "GITHUB https://github.com/riscv/riscv-qemu ff36f2f ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu"
                              "GIT https://git.qemu.org dtc 65cc4d27 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/dtc"
                              "GIT https://git.qemu.org SLOF e3d05727 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/SLOF"
                              "GIT https://git.qemu.org ipxe 04186319 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/ipxe"
                              "GIT https://git.qemu.org openbios c5542f22 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/openbios"
                              "GIT https://git.qemu.org openhackware c559da7c ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/openhackware"
                              "GIT https://git.qemu.org qemu-palcode c87a9263 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/qemu-palcode"
                              "GIT https://git.qemu.org seabios e2fc41e2 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/seabios"
                              "GIT https://git.qemu.org sgabios 23d47494 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/sgabios"
                              "GIT https://git.qemu.org u-boot 2072e726 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/u-boot"
                              "GIT https://git.qemu.org vgabios 19ea12c2 ./software/freedom-u-sdk/riscv-gnu-toolchain/riscv-qemu/roms/vgabios"
                              "GITHUB https://github.com/riscv/riscv-isa-sim 092b464 ./software/freedom-u-sdk/riscv-isa-sim"
                              "GITHUB https://github.com/riscv/riscv-pk 5a0e3e5 ./software/freedom-u-sdk/riscv-pk"
                              "GITHUB https://github.com/riscv/riscv-qemu ad8989a ./software/freedom-u-sdk/riscv-qemu"
                              "GIT https://git.qemu.org capstone 22ead3e ./software/freedom-u-sdk/riscv-qemu/capstone"
                              "GIT https://git.qemu.org keycodemapdb 6b3d716 ./software/freedom-u-sdk/riscv-qemu/ui/keycodemapdb"
                              "GIT https://git.qemu.org QemuMacDrivers d4e7d7a ./software/freedom-u-sdk/riscv-qemu/roms/QemuMacDrivers"
                              "GIT https://git.qemu.org SLOF fa98132 ./software/freedom-u-sdk/riscv-qemu/roms/SLOF"
                              "GIT https://git.qemu.org ipxe 0600d3a ./software/freedom-u-sdk/riscv-qemu/roms/ipxe"
                              "GIT https://git.qemu.org openbios 54d959d ./software/freedom-u-sdk/riscv-qemu/roms/openbios"
                              "GIT https://git.qemu.org openhackware c559da7 ./software/freedom-u-sdk/riscv-qemu/roms/openhackware"
                              "GIT https://git.qemu.org qemu-palcode f3c7e44 ./software/freedom-u-sdk/riscv-qemu/roms/qemu-palcode"
                              "GIT https://git.qemu.org seabios 0551a4b ./software/freedom-u-sdk/riscv-qemu/roms/seabios"
                              "GIT https://git.qemu.org seabios-hppa 1ef99a0 ./software/freedom-u-sdk/riscv-qemu/roms/seabios-hppa"
                              "GIT https://git.qemu.org sgabios cbaee52 ./software/freedom-u-sdk/riscv-qemu/roms/sgabios"
                              "GIT https://git.qemu.org skiboot e0ee24c ./software/freedom-u-sdk/riscv-qemu/roms/skiboot"
                              "GIT https://git.qemu.org u-boot d85ca02 ./software/freedom-u-sdk/riscv-qemu/roms/u-boot"
                              "GIT https://git.qemu.org u-boot-sam460ex 8ee007c ./software/freedom-u-sdk/riscv-qemu/roms/u-boot-sam460ex"
                              "GIT https://git.qemu.org vgabios 19ea12c ./software/freedom-u-sdk/riscv-qemu/roms/vgabios"
                              "#"
                              "# The following files are related to the generate of the Auto-FIR module"
                              "#"
                              "HTTP_FILE http://www.spiral.net/hardware firgen.tgz ./hdl_cores/auto-fir"
                              "HTTP_FILE http://spiral.ece.cmu.edu/mcm/dl synth-jan-14-2009.tar.gz ./hdl_cores/auto-fir"
                              )

# Function to "clone" a directory based on the specified repo
# $1 - A single entry from repo_names
clone_directory() {

  repo_type=$(echo " "$1 | cut -d' ' -f 2)

  case "$repo_type" in
    "#") 
    ;;
    "GITHUB")
      current_repo=$(echo " "$1 | cut -d' ' -f 3)
      current_commit=$(echo " "$1 | cut -d' ' -f 4)
      current_directory=$(echo " "$1 | cut -d' ' -f 5)
      echo
      echo "GITHUB: Exporting repository "$current_repo", Commit" $current_commit" to directory" $current_directory
      echo
      rm -f tmp.tar   # Ensure that the tempary tar does not already exist
      rm -rf $current_directory
      mkdir -p $current_directory
      curl -L $current_repo/tarball/$current_commit > tmp.tar

      # If curl failed to get something, a bogus tar file will be created resulted in a TAR failure
      if ! tar xf tmp.tar --strip-components=1 -C $current_directory; then
        echo "ERROR: curl: " $1
        exit 1
      fi
      rm -f tmp.tar
    ;;
    "GIT")
      current_site=$(echo " "$1 | cut -d' ' -f 3) 
      current_repo=$(echo " "$1 | cut -d' ' -f 4)
      current_commit=$(echo " "$1 | cut -d' ' -f 5)
      current_directory=$(echo " "$1 | cut -d' ' -f 6)
      echo
      echo "GIT: Exporting repository "$current_repo", Commit" $current_commit" to directory" $current_directory " from " $current_site
      echo
      rm -f tmp.tgz
      rm -rf $current_directory
      mkdir -p $current_directory      
      wget -O tmp.tgz "${current_site}/?p=${current_repo}.git;a=snapshot;h=${current_commit};sf=tgz"

      # If curl failed to get something, a bogus tar file will be created resulted in a TAR failure
      if ! tar zxf tmp.tgz --strip-components=1 -C $current_directory; then
        echo "ERROR: wget: " $1
        exit 1
      fi
      rm -f tmp.tgz              
    ;;
    "HTTP_FILE")
      current_url=$(echo " "$1 | cut -d' ' -f 3)
      current_file=$(echo " "$1 | cut -d' ' -f 4)
      current_directory=$(echo " "$1 | cut -d' ' -f 5)
      echo
      echo "HTTP: Downloading "$current_file" from "$current_url" to directory" $current_directory
      echo
      wget -O "${current_directory}/${current_file}" "${current_url}/${current_file}" 
    ;;
   esac
}

# Funtion to display the directory that will be overwritten
# $1 - A single entry from repo_names
list_directory() {
  repo_type=$(echo " "$1 | cut -d' ' -f 2)

  case "$repo_type" in
    "#")
    echo $1 
    ;;
  "GITHUB")
    echo " "$1 | cut -d' ' -f 5
    ;;
  "GIT")
    echo " "$1 | cut -d' ' -f 6
    ;;
  "HTTP_FILE")
    echo " "$1 | cut -d' ' -f 5
    ;;
  esac
}

# Funtion to append the directory to gitignore
# $1 - A single entry from repo_names
append_directory() {
  repo_type=$(echo " "$1 | cut -d' ' -f 2)

  case "$repo_type" in
    "#")
    ;;
  "GITHUB")
    echo " "${repo_names[$i-1]} | cut -d' ' -f 5 | tail -c +3 >> .gitignore
    ;;
  "GIT")
    echo " "${repo_names[$i-1]} | cut -d' ' -f 6 | tail -c +3 >> .gitignore
    ;;
  "HTTP_FILE")
    echo " "${repo_names[$i-1]} | cut -d' ' -f 5 | tail -c +3 >> .gitignore
    ;;
  esac
}

# "Main" function
number_of_repos=${#repo_names[@]}

echo ""
echo "------------------------------------------------------------------------------------------"
echo "---                       CEP Get External Dependencies Script                         ---"
echo "------------------------------------------------------------------------------------------"
echo " This script should ONLY be run from the root directory of your CEP clone.  Otherwise, it "
echo " may corrupt your CEP installation."
echo ""

# Check for tools to be installed 
if ! [ -x "$(command -v curl)" ]; then
    echo "Error: curl is not installed on your system.  Install using \"sudo apt install curl\""
    exit 1
fi
if ! [ -x "$(command -v wget)" ]; then
    echo "Error: wget is not installed on your system.  Install using \"sudo apt install wget\""
    exit 1
fi
if ! [ -x "$(command -v tar)" ]; then
    echo "Error: tar is not installed on your system.  Install using \"sudo apt install tar\""
    exit 1
fi

# Check that get_external_dependencies.sh exists in the current directory.  This is a bit of a safety check that
# the script is being run from the CEP root directory; then
if [ ! -f "get_external_dependencies.sh" ]; then
    echo "Error: script should be run from your CEP root directory."
    exit 1
fi

# Update / Create ALL of the dependencies
if [ "$1" == "all" ]; then
  echo
  echo " Target Directories (CAUTION: They will be overwritten):"
  echo
  for (( i=1; i<${number_of_repos}+1; i++ ));
  do
    list_directory "${repo_names[$i-1]}"
  done

  if [[ ${2^^} != "YES" ]]
  then
    echo
    read -p "Do you wish to continue? " -n 1 -r
    echo    # (optional) move to a new line
  fi

  if [[ $REPLY =~ ^[Yy]$ ]] || [[ ${2^^} == "YES" ]]
  then
    for (( i=1; i<${number_of_repos}+1; i++ ));
    do
      clone_directory "${repo_names[$i-1]}"
    done
  else
    exit 1
  fi

  echo
  echo "-----------------------------------------------------------------"
  echo "---       CEP Get External Dependencies Script Complete       ---"
  echo "-----------------------------------------------------------------"
  echo 

  exit 0

fi  # end if ALL option

if [ "$1" == "append" ]; then

  echo "Appending .gitignore..."

#  echo "# The following directories have been added to match those" >> .gitignore
#  echo "# created by the ./get_external_dependencies.sh script" >> .gitignore

  for (( i=1; i<${number_of_repos}+1; i++ ));
  do
    append_directory "${repo_names[$i-1]}"
  done

  echo
  echo "-----------------------------------------------------------------"
  echo "---       CEP Get External Dependencies Script Complete       ---"
  echo "-----------------------------------------------------------------"
  echo 

  exit 0

fi  # end of append option

# List ALL of the dependencies
if [ "$1" == "list" ]; then

  for (( i=1; i<${number_of_repos}+1; i++ ));
  do
    list_directory "${repo_names[$i-1]}"
  done

  echo
  echo "-----------------------------------------------------------------"
  echo "---       CEP Get External Dependencies Script Complete       ---"
  echo "-----------------------------------------------------------------"
  echo 

  exit 0

fi  # end if list option

# Perform a matching update: All modules that match the string
if [ "$1" == "matching" ]; then

  # Create an empty array
  declare -a match_indexes

  # Search through the list of repositories for a recursive substring (do not include
  # Do not include repos that are commented out
  for ((match_index=0; match_index<${number_of_repos}; match_index++)); do

    repo_type=$(echo " "${repo_names[$i-1]} | cut -d' ' -f 2)

    if [[ ${repo_names[$match_index]} == *$2* ]] && [[ ! $repo_type == "#" ]]; then
      match_indexes+=($match_index)
    fi
  done

  # If the match_indexes array length is zero, then no matches were found
  if [ ${#match_indexes[@]}  == 0 ]; then
    echo "No matching repository entry was found for \"$2\""
    echo
    
    exit 1  
  fi

  # A match was found, proceed to getting that specific module
  echo "The following directory(ies) will be overwritten"
  for i in "${match_indexes[@]}"; do
    list_directory "${repo_names[$i]}"
  done

  if [[ ${3^^} != "YES" ]]
  then
    echo
    read -p "Do you wish to continue? " -n 1 -r
    echo    # (optional) move to a new line
  fi

  if [[ $REPLY =~ ^[Yy]$ ]] || [[ ${3^^} == "YES" ]]
  then

    for i in "${match_indexes[@]}"; do
      clone_directory "${repo_names[$i]}"
    done
  else
    exit 1
  fi # end if $REPLY = Y?

  echo 
  echo "-----------------------------------------------------------------"
  echo "---       CEP Get External Dependencies Script Complete       ---"
  echo "-----------------------------------------------------------------"
  echo 

  exit 0

fi # end if matching option


echo "Usage:"
echo "  all                    - Get all external dependecies"
echo "  append                 - Append the list of all dependencies to .gitignore"
echo "  matching <module>      - Get all matching <module>"
echo "  list                   - List all external dependecies"
echo ""
echo "  For the options that ask for confirmation, specify a YES as the last"
echo "  argument will automatically confirm the request"
echo ""
exit 0
