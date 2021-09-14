#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX License Identifier: BSD-2-Clause
#//
#// File Name:      Makefile     
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Testbench makefile
#// Notes:          
#//	to run , type "make" 
#//
#//************************************************************************
#
LLKI_EN      = 1

#
# Applicable only if LLKI_EN=1
#
USE_MOCK_LLKI   = 1
CORE_WITH_LLKI = real_sha256_with_llki
#
# Add Cadence supports ro tun on RHEL7 machine
#
CADENCE  = 0
ifeq (${CADENCE},1)
DEFINE_LIST += +define+CADENCE
export CADENCE_ENABLE_AVSREQ_63188_PHASE_1:=1
export CADENCE_ENABLE_AVSREQ_44905_PHASE_1:=1
# also might need to set  as sudo
#sudo setsebool -P mmap_low_allowed 1
endif
#
#
ifeq (${LLKI_EN},0)
$(error *** LLKI_EN=0 no longer supported ***)
else
DUT_NAME     = sha256_mock_tss
endif

ifeq (${USE_MOCK_LLKI},1)
DEFINE_LIST += +define+USE_MOCK_LLKI
endif
#
# Use file from checked-in
#
CORE_VFILES = ../../hdl_cores/sha256/*.v ../../hdl_cores/sha256/*.sv -y ../../hdl_cores/llki

#
#
TB_NAME  = ${DUT_NAME}_tb
DEFINE_LIST += +define+DUT_NAME=${DUT_NAME} +define+TB_NAME=${TB_NAME}

#
#
#
all: sha256_stimulus.txt
ifeq (${CADENCE},1)
	xrun -64 -sv +acc *.sv ${LLKI_FILES} ${DEFINE_LIST} ${CORE_VFILES} -y ../llki_supports +libext+.v +libext+.sv +incdir+../llki_supports -xlm_bq1
else
	vlog -64 -sv +acc *.sv ${LLKI_FILES} ${DEFINE_LIST} ${CORE_VFILES} +libext+.v +libext+.sv
	vopt -64 +nolibcell +nospecify +notimingchecks ${TB_NAME} -o ${TB_NAME}_opt
	vsim -64 ${TB_NAME}_opt -batch -do vsim.do -logfile ${TB_NAME}.log -wlf ${TB_NAME}.wlf
endif

sha256_stimulus.txt : sha256_stimulus.txt.gz
	gunzip -c $< > $@

clean:
	rm -rf work *.wlf *.log *.txt 
	rm -rf xcelium.d


