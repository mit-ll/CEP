#!/usr/bin/python3
#//************************************************************************
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      amensia.py
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Renames verilog identifiers into random names
#// Notes:          
#//
#//************************************************************************

import dumper
import sys
import os
import re
import random
import string
import pyverilog
from optparse import OptionParser
from pyverilog.vparser.parser import parse
from pyverilog.ast_code_generator.codegen import ASTCodeGenerator
from pyverilog.utils.identifiervisitor import getIdentifiers

def stringGarbage( len):
    """returns len random alphabetic characters (a-z)."""
    garbage = ""
    for i in range(1, len):
        garbage = garbage + random.choice( string.ascii_lowercase)
    return garbage
        
        
def AST_traverse_rename_identifiers(ast, ids, attrnames=False):
    """Traverses the given AST, and replaces any identifiers it finds.
    
    ast -- target AST to modify
    ids -- from-to mapping; the keys are mapped to the values when found.
    attrnames -- names of any attributes in the AST object to modify
    """
    if hasattr(ast, 'name') and ast.name in ids:
        ast.name = ids[ast.name]
    if hasattr(ast, 'portname') and ast.portname in ids:
        ast.portname = ids[ast.portname]
    if hasattr(ast, 'argname') and ast.argname in ids:
        ast.argname = ids[ast.argname]

    for c in ast.children():
        AST_traverse_rename_identifiers(c, ids, attrnames)


def main():
    INFO = "Verilog identifier renamer"
    USAGE = "Usage: python3 amnesia.py file ..."

    keeplist = ["clk","reset","inData","outData"]

    def showUsage():
        print(INFO)
        print(USAGE)
        sys.exit()

    optparser = OptionParser()
    optparser.add_option("-I","--include",dest="include",action="append",
                         default=[],help="Include path")
    optparser.add_option("-D",dest="define",action="append",
                         default=[],help="Macro Definition")
    optparser.add_option("-x",dest="exclude",action="append",
                         default=keeplist,help="Identifiers to exclude from obfuscation")
    (options, args) = optparser.parse_args()

    filelist = args

    for f in filelist:
        if not os.path.exists(f): raise IOError("file not found: " + f)

    if len(filelist) == 0:
        showUsage()

    ast, directives = parse(filelist,
                            preprocess_include=options.include,
                            preprocess_define=options.define)

    #ast.show()
    # getIdentifiers is a list of all identifiers, including repeats.
    # ths i:i thing changes it on the fly into a dictionary,
    # as a way of doing "sort | uniq," basically.
    ids = { i : i for i in getIdentifiers( ast) }
    
    # now build our from:to mapping for all of those identifiers.
    for orig_id in ids:
        if orig_id in options.exclude:
            ids[orig_id] = orig_id
        else:
            ids[orig_id] = stringGarbage(32)

    ## PyVerilog comes with a function called "replaceIdentifiers" that should
    ## do this, but it doesn't get everything.  Ours will chomp on anything
    ## what has a name.
    AST_traverse_rename_identifiers(ast, ids)
        
    codegen = ASTCodeGenerator()
    rslt = codegen.visit( ast)  ## AST back to Verilog
    print( rslt)
        
if __name__ == '__main__':
    main()
