#!/usr/bin/python3
#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#// SPDX short identifier: MIT
#//
#// File Name:      auto-fir.py
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Peform a parameterized generation of a FIR/IIR filter
#//                 and subsequently generate and/or verify outputs
#// Notes:          
#//
#//************************************************************************

# Import Required packages
import os
import sys
import random
import argparse
import subprocess

# -------------------------------------------------------------------------
# Function      : cmd_exits
# Description   : A quick function to see if a command is available
# -------------------------------------------------------------------------
def cmd_exists(cmd):
    return subprocess.call("type " + cmd, shell=True, 
        stdout=subprocess.PIPE, stderr=subprocess.PIPE) == 0
# -------------------------------------------------------------------------
# End Function  : cmd_exists
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# Function      : get_sources
# Description   : Function to fetch and extract the tarball sources
# -------------------------------------------------------------------------
def get_sources(args):
    print (" One or both of the expected executables (./firgen/iirGen.pl, ./synth/acm) is missing")

    # Do we have the tarball sources or do we need to grab them?
    if not (os.path.isfile('./firgen.tgz') or os.path.isfile('./synth-jan-14-2009.tar.gz')):

        print(" Getting the tarball sources")

        # Save the current working directory
        cwd = os.path.dirname(os.path.realpath(sys.argv[0]))

        # Capture the repo's root directory
        repo_root = subprocess.run('git rev-parse --show-toplevel', check=True, shell=True, universal_newlines=True, stdout=subprocess.PIPE)

        # Change to the repo's root directory
        os.chdir(repo_root.stdout.rstrip())

        # Excute the get_external_depencies script to fetch the FIR generator source
        if args.verbose:
            os.system("./get_external_dependencies.sh matching auto-fir yes")
        else:
            os.system("./get_external_dependencies.sh matching auto-fir yes >> /dev/null 2>&1")

        # Return to the generared filter directory
        os.chdir(cwd)

    print(" Extracting tarballs and building the acm executable")

    # Perform the build (verbosely)
    if args.verbose:

        # Create the outputs directory
        os.system("mkdir -p outputs")

        # Extract the synth directory
        os.system("rm -rf ./synth")
        os.system("mkdir -p ./synth")
        os.system("tar -C ./synth --strip-components=1 -zxvf synth-jan-14-2009.tar.gz")

        # Patch a few files to allow compile
        os.chdir("./synth")

        os.system("sed -i '1i#include <cstdlib>' arith.h")
        os.system("sed -i '1i#include <algorithm>' bhm.cpp")

        # Build the mutliplier executable
        os.system("make acm")

        # Return back to the "original" directory
        os.chdir("..")

        # Extract the firgen
        os.system("rm -rf ./firgen")
        os.system("tar zxvf firgen.tgz")

    # Perform the build (non-verbosely)
    else: 

        # Create the outputs directory
        os.system("mkdir -p outputs >> /dev/null 2>&1")

        # Extract the synth directory
        os.system("rm -rf ./synth >> /dev/null 2>&1")
        os.system("mkdir -p ./synth >> /dev/null 2>&1")
        os.system("tar -C ./synth --strip-components=1 -zxvf synth-jan-14-2009.tar.gz >> /dev/null 2>&1")

        # Patch a few files to allow compile
        os.chdir("./synth")

        os.system("sed -i '1i#include <cstdlib>' arith.h >> /dev/null 2>&1")
        os.system("sed -i '1i#include <algorithm>' bhm.cpp >> /dev/null 2>&1")

        # Build the mutliplier executable
        os.system("make acm >> /dev/null 2>&1")

        # Return back to the "original" directory
        os.chdir("..")

        # Extract the firgen
        os.system("rm -rf ./firgen >> /dev/null 2>&1")
        os.system("tar zxvf firgen.tgz >> /dev/null 2>&1")
# -------------------------------------------------------------------------
# End Function  : get_sources
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# Function      : clean
# Description   : Perform some cleanup 
# -------------------------------------------------------------------------
def clean(args):

    if args.verbose:
        print("")
        print(" Removing the build/output directories")
        print("")

    if args.allclean:
        os.system("rm -f firgen.tgz")
        os.system("rm -f synth-jan-14-2009.tar.gz")

    os.system("rm -rf firgen")
    os.system("rm -rf synth")
    os.system("rm -rf outputs")
    os.system("rm -rf work")
    os.system("rm -rf __pycache__")
    os.system("rm -f transcript")
    os.system("rm -f *.firlog")
# -------------------------------------------------------------------------
# End Function  : all_sources
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# Function      : setup_parser
# Description   : Setup the parser and associated arguments
# -------------------------------------------------------------------------
def setup_parser():

    # Create the argument parser
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    group1 = parser.add_argument_group("General Arguments")
    group2 = parser.add_argument_group("Build Only Arguments")
    group3 = parser.add_argument_group("Simulation Only Arguments")
    group4 = parser.add_argument_group("Build and Simulation Arguments")

    # General arguments
    group1.add_argument("--clean", help="remove the build/output directories",
                        action="store_true")
    group1.add_argument("--allclean", help="remove the tarballs and the build/output directories",
                        action="store_true")
    group1.add_argument("-b", "--build", help="Generate the filter verilog",
                        action="store_true")
    group1.add_argument("-sg", "--simgen", help="Run the filter simulation in generate test vector mode",
                        action="store_true")
    group1.add_argument("-sv", "--simver", help="Run the filter simulation in verification mode",
                        action="store_true")
    group1.add_argument("-v", "--verbose", help="increase output verbosity",
                        action="store_true")

    # Build arguments
    group2.add_argument("-a", "--amnesia", help="Random verilog identifiers",
                        action="store_true")
    group2.add_argument("-seed", "--random_seed", type=int, default=-1,
                        help="Specify the PRNG seed (if not specified, system time will be used")
    group2.add_argument("-aminn", "--a_min_number", type=int, default=1,
                        help="Specify the minimum number of A coefficients")
    group2.add_argument("-amaxn", "--a_max_number", type=int, default=20,
                        help="Specify the maximum number of A coefficients")
    group2.add_argument("-bminn", "--b_min_number", type=int, default=1,
                        help="Specify the minimum number of B coefficients")
    group2.add_argument("-bmaxn", "--b_max_number", type=int, default=20,
                        help="Specify the maximum number of B coefficients")
    group2.add_argument("-aminv", "--a_min_value", type=int, default=1,
                        help="Specify the minimum possible A coefficient")
    group2.add_argument("-amaxv", "--a_max_value", type=int, default=2**25-1,
                        help="Specify the maximum possible A coefficient")
    group2.add_argument("-bminv", "--b_min_value", type=int, default=1,
                        help="Specify the minimum possible B coefficient")
    group2.add_argument("-bmaxv", "--b_max_value", type=int, default=2**25-1,
                        help="Specify the maximum possible B coefficient")

    # Simulation arguments
    group3.add_argument("-sam", "--samples", type=int, default=100,
                        help="Specify the number of samples to capture")
    group3.add_argument("-cf", "--capture_file", type=str, default="filter_tb_capture.firlog",
                        help="Specify the sample capture file")
    group3.add_argument("-log", "--log_file", type=str, default="filter.firlog",
                        help="Simulation log file")

    # Build and Simulation Arguments
    group4.add_argument("-dut", "--dut_name", type=str, default="filter",
                        help="Specify the DUT module name.  Expected verilog filename is <DUT>.v\n ")

    # Return the resulting data structure
    return parser.parse_args()
# -------------------------------------------------------------------------
# End Function  : setup_parser
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# Function      : run_build
# Description   : Generate the filter based on the specied argument
# -------------------------------------------------------------------------
def run_build(args):
    # Verify the presence of the iirGenn.pl and acm executables
    if not (os.path.isfile('./firgen/iirGen.pl') or os.path.isfile('./synth/acm')):
        get_sources(args)

    # Print the parameters
    if args.verbose:
        print("")
        print("Build Options -")
        print("  random_seed  = " + str(args.random_seed))
        print("  a_min_number = " + str(args.a_min_number))
        print("  a_max_number = " + str(args.a_max_number))
        print("  a_min_value  = " + str(args.a_min_value))
        print("  a_max_value  = " + str(args.a_max_value))
        print("  b_min_number = " + str(args.b_min_number))
        print("  b_max_number = " + str(args.b_max_number))
        print("  b_min_value  = " + str(args.b_min_value))
        print("  b_max_value  = " + str(args.b_max_value))
        print("")

    # Do some basic error checking on the parameters (to avoid breaking python OR perl)
    if (args.a_min_number > args.a_max_number or args.a_min_number > 20 or args.a_max_number > 20):
        sys.exit("ERROR: Both a_max_number and a_min_number must be <=20 AND min <= max")
    if (args.b_min_number > args.b_max_number or args.b_min_number > 20 or args.b_max_number > 20):
        sys.exit("ERROR: Both b_max_number and b_min_number must be <=20 AND min <= max")
    if (args.a_min_value > args.a_max_value or args.a_min_value > 2**25-1 or args.a_max_value > 2**25-1):
        sys.exit("ERROR: Both a_max_value and a_min_value must be <=2**25 AND min <= max")
    if (args.b_min_value > args.b_max_value or args.b_min_value > 2**25-1 or args.b_max_value > 2**25-1):
        sys.exit("ERROR: Both a_max_value and a_min_value must be <=2**25 AND min <= max")

    # Initialize the PRNG.  If the random_seed is non-negative, then
    # it will be used.  Otherwise, the system time will be used
    if (args.random_seed >= 0):
        random.seed(args.random_seed)
    else:
        random.seed()

    # How many A and B constants should we generate?
    a_number = random.randint(args.a_min_number, args.a_max_number)
    b_number = random.randint(args.b_min_number, args.b_max_number)

    # Initialize the A and B constant arrays
    a_constants = "1 "
    b_constants = ""

    # Generate the A constants
    for x in range(0, a_number):
        a_constants = a_constants + str(random.randint(args.a_min_value, args.a_max_value)) + " "

    # Generate the B constants
    for x in range(0, b_number):
        b_constants = b_constants + str(random.randint(args.b_min_value, args.b_max_value)) + " "    

    # Constructs the command string for calling the IIR Generator
    dut_filename   = "{}.v".format(args.dut_name) 
    command_string = "./iirGen.pl " + \
                     "-A " + a_constants + \
                     "-B " + b_constants + \
                     "-moduleName {} ".format(args.dut_name) + \
                     "-fractionalBits 0 " + \
                     "-bitWidth 64 " + \
                     "-inData inData " + \
                     "-inReg " + \
                     "-outReg " + \
                     "-outData outData " + \
                     "-clk clk " + \
                     "-reset reset " + \
                     "-reset_edge negedge " + \
                     "-filterForm 1 " + \
                     "-outFile ../outputs/" + \
                     dut_filename

    # Change to the firgen directory
    os.chdir("./firgen")

    # Call the IIR generator using the constructed command string
    if args.verbose:
        print(" Calling the IIR Generator using the following command -")
        print(command_string)
        os.system("time " + command_string)
    else:
        print(" Calling the IIR Generator")
        os.system(command_string)

    # Return to the original directory
    os.chdir("..")

    # Run amnesia on the resulting file randomize signal names
    if args.amnesia:

        amnesia_command = "./amnesia.py ./outputs/" + dut_filename
        if args.verbose:        
            print(" Running amnesia on the filter using the following command -")
            print(amnesia_command)
        else:
            print(" Running amnesia on the filter")

        with open("./outputs/tmp.v","w") as outfile:
            subprocess.run(amnesia_command, check=True, shell=True, stdout=outfile)
        os.system("cp ./outputs/tmp.v ./outputs/" + dut_filename)

        # Remove intermediate files
        os.system("rm -f ./outputs/tmp.v")
        os.system("rm -f parser.out")
        os.system("rm -f parsetab.py")

# -------------------------------------------------------------------------
# End Function  : run_build
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# Function      : run_questa_simulation
# Description   : Will run the simulation in either CAPTURE or VERIFY
#                 mode
# -------------------------------------------------------------------------
def run_questa_simulation(args):

    # Run simulation in verbose mode
    if args.verbose:
        # Using the IIR_filter_tb.v, generate a series of input/output pairs on the newly generated core
        # Remove the previous working directory
        os.system("rm -rf work")

        # Compile the newly generated core
        os.system("vlog ./outputs/{}.v".format(args.dut_name))

        # Compile the testbench in either CAPTURE or VERIFY mode
        if args.simgen:
            system_call = 'vlog +define+CAPTURE +define+NUM_SAMPLES={} +define+DUT_NAME={} '.format(args.samples, args.dut_name) + \
                          '+define+CAPTURE_FILE=\\\"{}\\\" filter_tb.v'.format(args.capture_file) 
            print("COMMAND: " + system_call)
            os.system(system_call)
        else:
            system_call = 'vlog +define+VERIFY +define+NUM_SAMPLES={} +define+DUT_NAME={} '.format(args.samples, args.dut_name) + \
                      '+define+CAPTURE_FILE=\\\"{}\\\" filter_tb.v'.format(args.capture_file) 
            print("COMMAND: " + system_call)
            os.system(system_call)

        # Define optimization options
        os.system("vopt +acc work.filter_tb -o dbugver")

        # Run the simulation
        os.system("vsim dbugver -classdebug +notimingchecks -c +trace_enable -do \"run -all;quit\" -l {};".format(args.log_file))

    # Run simulation in NOT verbose mode
    else:

        # Using the IIR_filter_tb.v, generate a series of input/output pairs on the newly generated core
        # Remove the previous working directory
        os.system("rm -rf work  >> /dev/null 2>&1")

        # Compile the newly generated core
        os.system("vlog ./outputs/{}.v  >> /dev/null 2>&1".format(args.dut_name))

        # Compile the testbench in either CAPTURE or VERIFY mode
        if args.simgen:
            system_call = 'vlog +define+CAPTURE +define+NUM_SAMPLES={} +define+DUT_NAME={} '.format(args.samples, args.dut_name) + \
                          '+define+CAPTURE_FILE=\\\"{}\\\" filter_tb.v  >> /dev/null 2>&1'.format(args.capture_file) 
            os.system(system_call)
        else:
            system_call = 'vlog +define+VERIFY +define+NUM_SAMPLES={} +define+DUT_NAME={} '.format(args.samples, args.dut_name) + \
                      '+define+CAPTURE_FILE=\\\"{}\\\" filter_tb.v >> /dev/null 2>&1'.format(args.capture_file) 
            os.system(system_call)

        # Define optimization options
        os.system("vopt +acc work.filter_tb -o dbugver  >> /dev/null 2>&1")

        # Run the simulation
        os.system("vsim dbugver -classdebug +notimingchecks -c +trace_enable -do \"run -all;quit\" -l {};".format(args.log_file))

# -------------------------------------------------------------------------
# End Function  : run_simulation
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# Function      : main
# Description   : Main function
# -------------------------------------------------------------------------

# Setup the argument parser
args = setup_parser()

if args.verbose:
    print ("")
    print ("------------------------------------------------------------------------------------------")
    print ("---                              CEP Filter Generator                                  ---")
    print ("------------------------------------------------------------------------------------------")
    print ("")

# Do some cleanup
if args.clean or args.allclean:
    clean(args)

# Run the generator, if specified so to do
elif args.build:
    run_build(args)

elif args.simgen or args.simver:    

    # Verify vsim is available
    if (not cmd_exists("vsim")):
        sys.exit("ERROR: vsim not available\n")

    # Call the questa-based simulation
    run_questa_simulation(args)

# end of if args.build
else:

    print ("")
    print ("Usage info - ./" + os.path.basename(__file__) + " -h")
    print ("")

if args.verbose:
    print ("")
    print ("------------------------------------------------------------------------------------------")
    print ("---                         CEP Filter Generator Complete                              ---")
    print ("------------------------------------------------------------------------------------------")
    print ("")

# -------------------------------------------------------------------------
# End Function  : main
# -------------------------------------------------------------------------
