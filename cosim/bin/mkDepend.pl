#!/usr/bin/perl
#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************
#
#  "makedepend" for verilog!!!
use POSIX;
use File::Basename;
use FileHandle;
use English;

# Get the program name
$program_name =  $0;

# Get the date 
$date = `date`; chop($date);

# Get a home
$home = $ENV{'HOME'} || $ENV{'LOGDIR'} ||
        (getpwuid($<))[7] || die "You're homeless!\n";

# Get my current working directory
$hostname = `hostname`; chop($hostname);

$my_cwd = `pwd`; chop($my_cwd);
# only if no "/net/" && ldisk
if ((!($my_cmd =~ /\/net\//)) && ($my_cmd =~ /ldisk/)) {
  $my_cwd = "/net/${hostname}${my_cwd}";
}
$error_found = 0;

############################
### Default for switches ###
############################
$DBG                    = 0;

$vmake_out_file         = @ARGV[0];
$build_dir              = @ARGV[1];
$target_name            = @ARGV[2];

#
$no_sort = 0;
#
$outfile                = "$build_dir/${target_name}.make";
$tmpfile                = "${outfile}.tmp";
print "target=$target_name out=$outfile tmp=$tmpfile\n" if $DBG;
#
# Database's init
#
%vc_list          = ();
%vhdl_list        = ();
$vc_empty = 1;
$vhdl_empty = 1;

#
# parse the VC file
#
&parse_log_file($vmake_out_file);
&print_output($tmpfile,$target_name);
# Check if $outfile exit them compare against $tmpfile
$thesame = 1;
if (-e $outfile) {
    open(CMD,"diff $outfile $tmpfile | wc -l|");
    while (<CMD>) {
      if (/\s*([1-9]+)/) {
	if ($DBG) { print "$outfile is recreated\n"; }
	$thesame = 0;
      }
    }
    close(CMD);
} else {
    $thesame = 0;
}
if (!$thesame) {
    `mv -f $tmpfile $outfile`;
} else {
    print "$outfile is unchanged\n";
}
`rm -f $tmpfile`;
exit $error_found ;

#
# output list of dependencies
#
sub print_output {
    local($outfile,$target_name) = @_;
    open(OFILE,">$outfile") || die "Can open output file $outfile\n";
    print OFILE "\${BLD_DIR}/$target_name : \${BLD_DIR}/${target_name}_OTHER_LIST \${BLD_DIR}/${target_name}_VHDL_LIST";
    print OFILE "\n\ttouch \$@\n";
    print OFILE "\n";        
    # VHDL
    print OFILE "\${BLD_DIR}/${target_name}_VHDL_LIST : ";
    if (!$vhdl_empty) {
	foreach $item (sort keys(%vhdl_list)) {
	    print OFILE " \\\n\t\t${item}";
	}
    }
    print OFILE "\n\ttouch \$@\n";
    print OFILE "\n";
    # Verilog and the rest
    print OFILE "\${BLD_DIR}/${target_name}_OTHER_LIST : ";
    if (!$vc_empty) {
	foreach $item (sort keys(%vc_list)) {
	    print OFILE " \\\n\t\t${item}";
	}
    }
    #
    print OFILE "\n\ttouch \$@\n";
    print OFILE "\n";    
    close(OFILE);
}

#
# Parse the simv build log file and look for following key words: 
#
# Compiling <vfile>
# Compiling include file <ifile>
# Scanning library directory file <vfile>
#
#
sub parse_log_file {
    local($vmake_out_file) = @_;
    open(VMAKE_OUT_FILE,"$vmake_out_file") || die "%vmake_out_file not found or can not opened";
    # look for any file with suffix .v .vhd* .inc* .s* .h* etc
    while (<VMAKE_OUT_FILE>) {
	@tmp = split(/[:=\t\n ]+/,$_);
	for ($i=0;$i<=$#tmp;$i++) {
	    if (-f $tmp[$i] && !($tmp[$i] =~ /searchPaths_build/)) {
		$testName = basename($tmp[$i]);
		# add build_dir if single !!
		if ($testName eq $tmp[$i]) {
		    print "Adding fullpath to $tmp[$i]\n";
		    $testName = "$build_dir/$testName";
		} else {
		    $testName = $tmp[$i];
		}
		if ($testName =~ /\.vhd/) {
		    $vhdl_list{$testName} = 1;
		    $vhdl_empty = 0;
		} else {
		    $vc_list{$testName} = 1;
		    $vc_empty = 0;		    
		}
		print "Got $tmp[0]\n" if $DBG;
	    }
	}
    }	    
    close(VMAKE_OUT_FILE);
}
#printf "$program_name $date $home $my_cwd $vmake_out_file\n";
exit $error_found ;
