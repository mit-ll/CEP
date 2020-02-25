#!/usr/bin/perl
#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

$verilog_file = @ARGV[0];
$c_file       = @ARGV[1];

#
# Open verilog file
#
open (VP,"$verilog_file") || die "Can't open VERILOG file $verilog_file";
open (CP,">$c_file") || die "Can't open C file $c_file";

while(<VP>) 
{
    if (/^\/\// ||
	/^\s*$/) 
    {
	print CP $_; # keep the comment
    }
    elsif (/^\s*`ifdef/ ||
	   /^\s*`ifndef/ ||	   
	   /^\s*`else/ ||
	   /^\s*`endif/) {
	$_ =~ s/`/#/;
	print CP $_;
    }
    # found a `include = comment it out for now
    elsif (/^\s*`include\s*/) {
	$_ =~ s/`include/\#include/; # replace .incl with .h
	$_ =~ s/\.incl/\.h/; # replace .incl with .h
	print CP $_;
    }

    # found a define by itself
    elsif (/^\s*`define\s+(\w+)\s*$/) {
	$_ =~ s/`/#/;
	print CP $_;
    }
    # found a hexadecimal `define
    elsif (/^\s*`define\s+(\w+)\s+[0-9]{0,2}'h([0-9a-fA-F_]+)/) {
	($a,$b,$c) = ($1,$2,$3);
	if (length(&remove_underscores($b))>8) {
	  printf(CP "#define %-20s 0x%sULL\n",$a,&remove_underscores($b))     ;
	} else {
	  printf(CP "#define %-20s 0x%s\n",$a,&remove_underscores($b))     ;
	}
    }
    # found a binary `define
    elsif (/^\s*`define\s+(\w+)\s+[0-9]{0,2}'b([01_xX]+)\s*(\/\/.*)?/) {
	($a,$b,$c) = ($1,$2,$3);
        $b = &remove_underscores($b);
        $b = &bin2decimal($b);
	printf(CP "#define %-20s 0x%08x\n",$a,&remove_underscores($b))     ;
    }
    # found a decimal `define
    elsif (/^\s*`define\s+(\w+)\s+([0-9]{0,2}'d)?([0-9_]+)\s*(\/\/.*)?/) {
	($a,$b,$c,$d) = ($1,$2,$3,$4);
	printf(CP "#define %-20s	 %s\n",$a,&remove_underscores($c));
    }
    # others
    elsif (/^\s*`define\s+([\S]+)\s*(.*)/) {
	($a,$b) = ($1,$2);
	$b =~ s/`//g;                              ;# if recursive, replace "`" with ""
        $b =~ s/32'h/0x/g;                         ;  # replace "32'h" with "0x"
        #$b = &remove_underscores($b);
	printf(CP "#define %-20s   %s\n",$a,$b);            ;#generate a tcl statement
    }
    else {
	print CP $_; # keep the comment
    }	   
}

close(VP);
close(CP);
exit 0;

sub bin2decimal {
local($b) = @_;
$b = reverse($b);
local($bit,$d);
while($b ne "")
  {
    $bit = chop($b);
    $d = $d << 1;
    $d = $d + 1 if( $bit eq "1" );
  }
return $d;
}

sub remove_underscores {
local($y) = @_;
$y =~ s/_//g;	  ;# eliminate underscores
$y =~ s/[xX]/0/g; ;# map x's to 0
return $y;
}
