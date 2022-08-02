#!/usr/bin/env bash
#//--------------------------------------------------------------------------------------
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File          : program_vcu118_jtag.sh
#// Project       : Common Evaluation Platform (CEP)
#// Description   : Shell script for invoking program_vcu118_jtag.tcl
#// Notes         : exit script if any command fails
#//--------------------------------------------------------------------------------------
set -e
set -o pipefail

vivado -mode tcl -source program_vcu118_jtag.tcl
