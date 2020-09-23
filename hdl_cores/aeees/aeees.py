#!/usr/bin/python3
#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#// SPDX short identifier: MIT
#//
#// File Name:      aeees.py
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Main python script for the Advaced Egregiously 
#//                 Extended Encryption Standard, White-box edition
#//
#// Notes:          
#//
#//************************************************************************

import sys
import os
import glob
import argparse
import aeees_gen
import aeees_wb_gen
from jinja2 import Environment, Template, FileSystemLoader

## AEEES.py
#
# Generates an AES-128 core with a variable number of rounds.  If you
# request a 10-round core, the result computes AES-128 as expected;
# more or fewer rounds repeat the AES round function and key schedule
# according to the specification, but are no longer standard AES.
#
# If you specify a key on the command line, it generates a "whitebox"
# variant that bakes the key into the S-Boxes for each round, and
# modifies the module I/O so the key parameter is no longer requested.

# command-line argument parser, adapted from filter generator
def setup_parser():

    # Create the argument parser
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)

    modegroup = parser.add_mutually_exclusive_group(required=True)
    modegroup.add_argument("-k", "--key", default=None, help="To enable whitebox core generation, specify the 128-bit key embed into the implementation")
    modegroup.add_argument("-nk", "--nokey", default=False, action="store_true", help="To perform non-whitebox core generation")
    modegroup.add_argument("--clean", default=False, action="store_true", help="Will remove ALL UNTRACKED FILES and DIRECTORIES starting in the current directory")    

    parser.add_argument("-d", "--dirname", default=".", help="Specify the directory into which files should be placed when generated.")
    parser.add_argument("-r", "--rounds", type=int, default=10, help="Specify the number times to run the AES round function (10 = stock AES-128)")
    parser.add_argument("-n", "--dut_name", default="aeees", help="Specify the module name (default aeees).  Expected output filenames are <DUT>.v, <DUT>_tb.sv, and <DUT>_stimulus.csv.")
    parser.add_argument("-t", "--trials", type=int, default=10, help="Specify the number of trials to build into the automated testbench (default 10).")
    parser.add_argument("-v", "--verbose", default=False, action="store_true")

    # Return the resulting data structure
    return parser.parse_args()

## MAIN
args = setup_parser()

if args.clean:
    victims = [ 'aeees.v', 'aeees_rounds.v', 'aeees_table.v', 'aeees_stimulus.csv', 'aeees_tb.sv', 'Makefile', 'vsim.do', 'aeees_roundfn.vi' ] + glob.glob('aeees_box_*.vi', recursive=False)
    for delpat in victims:
        if args.verbose:
            print("Deleting %s" % delpat)
        try:
            os.remove( delpat)
        except:
            ()

    sys.exit("Cleanup complete.")

if args.verbose:
    print( """
---------------------------------------------------------------------
---                       A(EE)ES Generator                       ---
---------------------------------------------------------------------
""")

# check for what we need
whereami = os.path.join( os.path.dirname(os.path.realpath(sys.argv[0])), "templates" )
for tmpname in [ 'aeees_gen.t', 'aeees_tb.t', 'rounds.t', 'table.t', 'aeees_wb_gen.t', 'aeees_wb_tb.t', 'rounds_wb.t', 'fnsbox_wb.t', 'roundfn_wb.t', 'Makefile.t', 'vsim.do.t' ]:
    if( not os.path.isfile( os.path.join(whereami, tmpname))):
        sys.exit( "ERROR:  The associated template file \'%s\' could not be found; aborting" % tmpname)

if( args.rounds < 10 ):
    if args.verbose:
        print("WARNING:  Requested rounds is below minimum of ten; will generate 10 rounds")
    args.rounds = 10
if( args.rounds > 32000 ):
    if args.verbose:
        print("WARNING:  Requested rounds is above maximum of 32,000; will generate 32,000 rounds")
    args.rounds = 32000

if( args.trials < 1 ):
    sys.exit("ERROR:  Cannot generate an empty testbench; specify trials >= 1")
if( args.trials > 9999 ):
    sys.exit("ERROR:  That's an unreasonable number of test cases; specify trials < 10,000")
    

env = Environment( loader = FileSystemLoader(whereami))

if( args.key is None ):  # "stock" AES
    aeees_gen.aeees_gen( args, env )
else:  # "whitebox" AES
    aeees_wb_gen.aeees_wb_gen( args, env )

if args.verbose:
    print("Generating Makefile")

template = env.get_template( 'Makefile.t')
with open( os.path.join( args.dirname, "Makefile"), "w") as fh:
    print( template.render(dut_name = args.dut_name ), file=fh)

if args.verbose:
    print("Generating vsim.do")
template = env.get_template( 'vsim.do.t')
with open( os.path.join( args.dirname, "vsim.do"), "w") as fh:
    print( template.render(dut_name = args.dut_name ), file=fh)

if args.verbose:
    print("Done!")
