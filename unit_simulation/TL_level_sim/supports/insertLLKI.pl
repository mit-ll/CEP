#!/usr/bin/perl
#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#//
#// File Name:      insertLLKI.pl
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

use POSIX;
use File::Basename;
use FileHandle;
use English;

#
# arguments
#
$in_file         = @ARGV[0];
$out_file        = @ARGV[1];

$debug = 0;
#
#
#
open(FP,$in_file) || die "Can't find file $in_file";
open(OF,">$out_file") || die "Can't find file $out_file";
print(OF "`include \"llki_struct.h\"\n");
while (<FP>) {
    $newline = $_;    
    if (/^module TL([^(]+)/) {
	#printf("Found $1 from : $_");
	$mod = "TL" . $1;	
	if (($1 =~ /Monitor/) || ($1 =~ /Buffer/)) {
	    #printf("Skip $_");
	} else {
	    $newline =~ s/$mod\(/${mod}_mock #(parameter llki_s MY_STRUCT) \(tlul_if\.slave tlul, /g;
	    #printf("After $newline");
	}
    } 
    elsif (/([\S]+)\s+blackbox\s+\(/) {
	#printf("$1\n");
	$modName = $1;
	$newMod  = $1 . "_mock_tss";
	print(OF "llki_discrete_if discreteIf();\n");
	# use buffer_clock and reset since core might have active-low reset!!!
	print(OF "llki_pp_mock #(.MY_ID(MY_STRUCT.core_id)) llki_pp(.clk(buffer_clock), .rst(buffer_reset),.discrete(discreteIf.master),.tlul(tlul));\n");
	$newline =~ s/blackbox \(/blackbox \( \.llki\(discreteIf.slave\), /g;
	$newline =~ s/$modName/$newMod #(.MY_STRUCT(MY_STRUCT))/;	
    }
    print(OF $newline);
}
close(FP);
close(OF);
