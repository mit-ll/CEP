#!/usr/bin/perl
#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX License Identifier: BSD-2-Clause
#//
#// File Name:      vpp.pl
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
$c_file         = @ARGV[0];
$Random         = @ARGV[1];
$updateInfo     = @ARGV[2];
$infoFile       = @ARGV[3];
$use_gdb        = @ARGV[4];
$logFile        = @ARGV[5];
$coverage       = @ARGV[6];
$vsim_cmd       = @ARGV[7];


#printf("plusArg=%s\n",$plusargs);
$debug = 0;
printf("c_file=%s Random=%d updateInfo=%d infoFile=%s gdb=%d\n",$c_file,$Random,$updateInfo,$infoFile,$use_gdb) if $debug;
printf("vsim_cmd=%s\n",$vsim_cmd) if $debug;

#
# Check Platform
#
$platform = `uname`; chop($platform) ;
#
# file contain perl code to filter test.log
#
$ignoreFileName = "ignoreMessages";
$useOldSeed  = 0;
$intTaken    = 0;
$debug       = 0;
$testDir     = dirname($c_file);
$testName    = basename($testDir);
#
printf("use_gdb flags = $use_gdb\n");
#
# first read the infoFile and get the last seed and its status
#
$seed = 0;
$lastFail = 1;
$msg = "";
#printf("Argument count = %d\n",$#ARGV);
if ($Random) {
    $new_seed = `date +%M%H%d%m`; chop($new_seed); 
    $new_seed += $PROCESS_ID;
    $seed = hex($new_seed);
    if (-e $infoFile) {
	open(FP,$infoFile) || die "Can't find file $infoFile";
	# get the first seed and its status
	$notFound = 1;
	$seed_stat = "FAIL";
	while (<FP>) {
	    if ($notFound && /^SEED\s+([0-9A-Fa-f]+)\s+STATUS\s+([\S]+)/) {
		$old_seed   = $1;
		$seed_stat  = $2;
		$notFound   = 0;
	    }
	}
	close(FP);
	printf("old_seed=$old_seed status=$seed_stat updateInfo=$updateInfo notFound=$notFound\n");
	if ($useOldSeed) { 
	    $seed = hex($old_seed);
	    $msg = "(OLD SEED)";
	    printf("Using old seed=0x%x\n",$seed);
	} 
	elsif ($notFound || (($seed_stat eq "PASS") && $updateInfo)) {
	    $lastFail  = 0;
	    $seed = hex($new_seed);
	    $msg = "(NEW SEED)";
	    printf("Using new seed=0x%x\n",$seed);
	} else {
	    $seed = hex($old_seed);
	    $msg = "(OLD SEED)";
	    printf("Using old seed=0x%x\n",$seed);
	}
    } else {
	# create testInfo file and add in CVS
	$seed = hex($new_seed);
	$msg = "(NEW SEED)";
	printf ("WARNING: testInfo file $infoFile does not exit..Creating one..\n");
	system("touch $infoFile");
    }
}

# no output buffer
$OUTPUT_AUTOFLUSH = 1;  # $| piping hot
#
$parent_pid = $PROCESS_ID; # $$
printf ("My process ID is %d\n",$parent_pid) if $debug;

#
# FORK Code
#
%childpidList = ();
$numberOfChild = 1; # + $dsimEnable;	
$notDone = 1;
$childNum = 0;
system("rm -f *.KEY");
while ($notDone && ($childNum < $numberOfChild)) {
  if ($childNum == 0) {
    $simFile = $vsim_cmd;
  } else {
    $simFile = "${vsim_cmd}_slave$childNum";
  }
  if ($pid = fork) {
    # get here if parent
    # to to child List
    $childpidList{$pid} =  $simFile;
    $childNum++; # next
  } elsif (defined $pid) { # $pid is 0 if defined
    # get here if child
    &run_child_cmdsf($simFile);
  } elsif ($ERRNO =~ /No more process/) {
    # supposely fork error
    sleep 5;
    die "No more process\n";
    $notDone = 0;
  } else {
    # wired fork error
    $notDone = 0;
    die "Can't fork: $ERRNO\n";
  }
}
#
# If get here all child are forked OK
#
&run_caller_cmds();
#
# Kill all child process
#
sub kill_all_threads {
  local($hardKill) = @_;
  sleep(3);
  if ($updateInfo) {
      printf("Going to sleep for 20 seconds for the simv to finish cleaning up like coverage dumping\n");
      if ($coverage) {
	  sleep(20);
      }
  }
  foreach $pid (keys(%childpidList)) {
    $killCmd = "kill $hardKill $pid";
    printf("kill_all_threads::Terminating Simv process $childpidList{$pid} with commad=$killCmd\n");
    system($killCmd);
  }
}

#
# For child
#
sub run_child_cmdsf {
  local($simFile) = @_;
  #
  # Batch Slave if enable
  #
  #  $plusargs =~ s/\"//g;
  $simv_command_line =  "$simFile";
  $simv_command_line =~ s/\"//g;
  #
  $myPID = $PROCESS_ID;
  printf("From Child PID=$myPID EXEC: $simv_command_line\n");
  system("ulimit -c 0");
  exec($simv_command_line);
  $parent_pid = getppid();
  printf("From Child PID=$myPID EXEC return status=%x Parent_ID is = %d\n",$OS_ERROR,$parent_pid);
  printf("run_child_cmdsf::Terminating process=$parent_pid\n");
  exit;
}
#
# For Parent
#
sub run_caller_cmds {
    #
    # Catch interrupt
    # 
    $SIG{INT}   = "signalHandler";
    $SIG{QUIT}  = "signalHandler";
    $SIG{TERM}  = "signalHandler";

    $myPID = $PROCESS_ID;
    printf("From Parent PID=$myPID\n");
    #
    # call my C_test here and do a wait on either process
    #
    `echo "... RUN  : $testDir $msg" > status`;
    $cTest_cmd = sprintf("$c_file 0x%x",$seed);
    $stat = 0;
    if ($use_gdb) {
       # create GDB cmd file
      open (GF,">gdbCmdFile");
      printf (GF "run %x\n",$seed);
      close(GF);
      $gdb_cmd = "gdb -x gdbCmdFile $c_file";
      print "Running GDB with: $gdb_cmd\n";
      $stat = system($gdb_cmd);
    } else {
      print "Executing $cTest_cmd\n";
      $stat = system($cTest_cmd);
    }
    #
    # Check if test finish successfully
    #
    printf("run_caller_cmds::Status return is %x\n",$stat);
    if (($stat != 0xD) && ($stat)) {
	$errCnt = 1;
    }
    # first check if there is any thing to filter in the test.log
    $doFilter = 0;
    if (-e $ignoreFileName) {
      $doFilter = 1;
      # force it to read the whole thing into one string
      $/ = undef; 
      open(INFO,$ignoreFileName);
      $FilterList = <INFO>;
      close(INFO);
      # back to default
      $/ = "\n";
    }
    #
    # Check test.log for "ERROR" message
    #
    open (TL,"egrep -i '(\s+ERROR|FATAL)' $logFile |");
    while (<TL>) {
      if ($doFilter) {
        $ignoreFlag = 0;
	#printf("FILTER:$_");
	eval $FilterList;
	if ($ignoreFlag) {
	  printf("Ignored: $_");
	} else {
	  printf("NoFiltered: $_");
	  $errCnt++;
	}
      } else {
	print $_;
	$errCnt++;
      }
    }
    close(TL);
    #
    #
    #
    open (TL,"grep WARN $logFile |");
    while (<TL>) {
	print $_;
    }
    close(TL);
    printf("run_caller_cmds::Total errCnt is %d\n",$errCnt);
    #
    # Clean up
    #
    &kill_all_threads();
    #
    # update the testInfo file only in updateInfo
    #
    if ((!$lastFail && $errCnt) || ($updateInfo && $Random)) {
	# do a cvs update just incase
	system("grep SEED $infoFile > ${infoFile}.tmp");
	open(FP,">$infoFile") || die "Can't find file $infoFile";
	$date = `date`;chop($date);
	printf (FP "SEED %x STATUS %s $date\n",$seed,$errCnt ? "FAIL" : "PASS");
	close(FP);
	system("cat ${infoFile}.tmp >> $infoFile; rm -f ${infoFile}.tmp");
    }
    #
    # return to Make
    #
    if ($errCnt) {
	`echo "*** FAIL : $testDir $msg" > status`;
	exit(-1);
    }
    else {
	`echo "    PASS : $testDir $msg" > status`;
	exit;
    }
}

#
# If I got kill
#
sub signalHandler {
    my $signame = shift;
    my $command;
    
    print "
********************************************
  SIGNAL HANDLER CALLED
********************************************
";
    printf " Signal Name is $signame\n";
    if ($intTaken) {
	printf "INTTERRUPT Previously taken.. No need to process it more then 1\n";
    } else {
	$intTaken++;
	printf "intTaken=$intTaken\n";
	# do this only if I am parent??
	$myPID = $PROCESS_ID;
	printf("My Process ID is $myPID\n");
	if ($myPID == $parent_pid) {
	    &kill_all_threads(-9);
	    #
	    # update the testInfo file only in updateInfo
	    #
	    $errCnt = ($stat) ? 1 : 0;
	    #
	    # Check test.log for "ERROR" message
	    #
	    printf("run_caller_cmds::Status return is %x\n",$stat);
	    open (TL,"grep ERROR $logFile");
	    while (<TL>) {
		print $_;
		$errCnt++;
	    }
	    close(TL);
	    printf("run_caller_cmds::Total erCnt is %d\n",$errCnt);
	    if ((!$lastFail && $errCnt) || ($updateInfo && $Random)) {
		# do a cvs update just incase
		system("grep SEED $infoFile > ${infoFile}.tmp");
		open(FP,">$infoFile") || die "Can't find file $infoFile";
		$date = `date`;chop($date);
		printf (FP "SEED %x STATUS %s $date\n",$seed,$errCnt ? "FAIL" : "PASS");
		close(FP);
		system("cat ${infoFile}.tmp >> $infoFile; rm -f ${infoFile}.tmp");
	    }
	} else {
	    printf("$myPID=kid => Parent suppose to handle the cleanup :-)))))\n");	
	}
	#
	# return to Make
	#
	`echo "*** FAIL : $testDir $msg" > status`;
    }
    exit(-1);
}

