#!/bin/sh
#//************************************************************************
#// Copyright (C) 2018 Massachusetts Institute of Technology
#//
#// File Name:      run_regression.sh
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Shell script to run CEP regression test suite
#// Notes:          CEP_version is set to the current CEP release
#//                 QUESTA_version is set to the version of Questa Sim used
#//                    to validate the regression test suite
#//
#//************************************************************************

CEP_version='v1.2'
QUESTA_version='10.7c'

# Check to see if QuestaSim is installed 
if !(command -v vsim > /dev/null); then
    echo "Error: Questa Sim-64 is not installed"
    exit
fi

# Measure the Start Time
START=$(date +%s.%3N)

echo ""
echo "==================================================================================="
echo "                           CEP $CEP_version Regression Test Suite"
echo "==================================================================================="
echo ""

# Initialize testResults
declare -i totalTests=0
declare -i totalTestsResults=0
declare -i testResult=0

for fn in `ls run_sim*.do | grep -v '^run_sim_CEP.do'`; do
    echo "Running $fn..."

    # Increment the total number of tests 
    totalTests="$(expr $totalTests + 1)" 

    # Execute the test and capture the results
    (vsim -c -do $fn)   
    if (($? > 0));then
        ((totalTestsResults++))
    fi

done

# Measure the end time and take the difference
END=$(date +%s.%3N)
DIFF=$(echo "$END - $START" | bc)

echo ""
echo "==================================================================================="
echo "                       CEP $CEP_version Regression Test Suite Results"
echo "==================================================================================="
echo ""
echo "                 Total tests run      = $totalTests"
echo "                 Total tests failed   = $totalTestsResults"
echo "                 Total execution time = $DIFF seconds"
echo "                 Test list ="
ls run_sim*.do | grep -v '^run_sim_CEP.do'
echo ""
echo "==================================================================================="
if !(vsim -version | grep "Questa Sim-64 vsim $QUESTA_version") then
    echo "    Warning: Regression suite verified using Questa Sim-64 $QUESTA_version"
    echo "    Installed version: `vsim -version`"
fi
echo ""
