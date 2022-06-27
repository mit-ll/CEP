#!/usr/bin/perl
#//************************************************************************
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX License Identifier: BSD-2-Clause
#//
#// File Name:      createPassFail.pl
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
$in_file  = @ARGV[0];
$out_file        = @ARGV[1];

$debug = 0;
#
#
#
$foundPass = 0;
$foundFail = 0;
$foundEcall = 0;
open(FP,$in_file) || die "Can't find file $in_file";
open(OF,">$out_file") || die "Can't find file $out_file";
while (<FP>) {
    chop($_);
    if (/^([0-9,a-f,A-F]+) <test_pass>\:/) {
	printf("Found $1 from : $_\n") if $debug;
	print(OF "\@0 $1 // $_\n");
	$foundPass = 1;
    }
    elsif (/^([0-9,a-f,A-F]+) <test_fail>\:/) {
	printf("Found $1 from : $_\n") if $debug;
	print(OF "\@1 $1 // $_\n");
	$foundFail = 1;
    }
    elsif (/^([0-9,a-f,A-F]+) <pass>\:/) {
	printf("Found $1 from : $_\n") if $debug;
	print(OF "\@0 $1 // $_\n");
	$foundPass = 1;
    } elsif (/^([0-9,a-f,A-F]+) <fail>\:/) {
	printf("Found $1 from : $_\n") if $debug;
	print(OF "\@1 $1 // $_\n");
	$foundFail = 1;
    } elsif (/^([0-9,a-f,A-F]+) <finish>\:/) {
	printf("Found $1 from : $_\n") if $debug;
	print(OF "\@2 $1 // $_\n");
	$foundPass = 1;
    } elsif (/^([0-9,a-f,A-F]+) <write_tohost>\:/) {
	printf("Found $1 from : $_\n") if $debug;
	print(OF "\@3 $1 // $_\n");
    } elsif (/^([0-9,a-f,A-F]+) <hangMe>\:/) {
	printf("Found $1 from : $_\n") if $debug;
	print(OF "\@4 $1 // $_\n");
    } elsif (/^\s+([0-9,a-f,A-F]+):\s+([0-9,a-f,A-F]+)\s+ecall/) {
	printf("Found $1 from : $_\n") if $debug;
	$foundEcall = $1;
    }
    print(OF $newline);
}
#
if (($foundPass == 0) && ($foundFail==0) ) {
    print(OF "\@0  $foundEcall // Ecall\n");
    printf(OF "\@1  %x // next\n",hex($foundEcall)+4);
}
close(FP);
close(OF);
