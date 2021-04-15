#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      common.make
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

#
# ----------------------------------------
# Every thing you need to know and do related to Cadence tool set are in here
# ----------------------------------------
#
export VMGR_VERSION	?= VMANAGERAGILE20.06.001
export XCELIUM_VERSION	?= XCELIUMAGILE20.09.001

export VMGR_PATH	?= /brewhouse/cad4/x86_64/Cadence/${VMGR_VERSION}
export XCELIUM_INSTALL  ?= /brewhouse/cad4/x86_64/Cadence/${XCELIUM_VERSION}


export IMC_INSTALL      ?= ${VMGR_PATH}
export MDV_XLM_HOME     ?= ${XCELIUM_INSTALL}
#
# Add to PATH if they are not there yet
#
XCELIUM_IN_PATH = $(shell echo $PATH | grep '$(XCELIUM_VERSION)')
IMC_IN_PATH     = $(shell echo $PATH | grep '$(VMGR_VERSION)')
#
ifeq ($(XCELIUM_IN_PATH),)
export PATH             := ${XCELIUM_INSTALL}/tools/bin:${XCELIUM_INSTALL}/tools/dfII/bin:${PATH}
endif
ifeq ($(IMC_IN_PATH),)
export PATH             := ${IMC_INSTALL}/tools/bin:${PATH}
endif
#
#
#
XRUN_CMD = ${XCELIUM_INSTALL}/tools/bin/xrun
IMC_CMD  = ${IMC_INSTALL}/tools/bin/imc
#
SAHANLDER_FILE 		= ${DUT_TOP_DIR}/cosim/sahandler.c
#
# If coverage is on
#
ifeq (${COVERAGE},1)
CADENCE_COV_COM_ARGS += -covfile ${DUT_TOP_DIR}/cosim/cadence_cov.ccf
CADENCE_COV_RUN_ARGS += -write_metrics -covoverwrite -covworkdir ${DUT_COVERAGE_PATH} -covscope ${TEST_SUITE} -covtest ${TEST_NAME} 
endif

# build the library for Cadence via this command under vivado
# cd <...>/cosim
# vivado -mode tcl
# compile_simlib -simulator xcelium -simulator_exec_path {$XCELIUM_INSTALL/tools/bin} -family all -language all -library all -dir {./cad_xil_lib} -force -verbose
CADENCE_XIL_LIB         := ${DUT_TOP_DIR}/cosim/cad_xil_lib
#
# something need to override
#
DUT_XILINX_VLOG_ARGS	+= +define+CADENCE
#
# override the VPP command for Cadence tool
#
override VSIM_CMD_LINE = "${XRUN_CMD} -64bit -R -xmlibdirname ${BLD_DIR}/xcelium.d  -afile ${PLI_DIR}/v2c.tab -loadpli1 ${DLL_DIR}/libvpp.so -sv_lib ${DLL_DIR}/libvpp.so -loadvpi ${BLD_DIR}/xcelium.d/run.d/librun.so:boot ${CADENCE_COV_RUN_ARGS} "


${BLD_DIR}/.cadenceBuild : ${VERILOG_DEFINE_LIST} ${SIM_DIR}/common.make ${SIM_DIR}/cep_buildChips.make ${DUT_XILINX_TOP_FILE} ${BLD_DIR}/${SIM_DEPEND_TARGET}_OTHER_LIST ${BHV_DIR}/ddr3.v
	$(RM) ${BLD_DIR}/searchPaths_build
	@for i in ${SEARCH_DIR_LIST}; do                        			\
		echo "-y" $${i} >> ${BLD_DIR}/searchPaths_build;			\
	done
	@for i in ${INCDIR_LIST}; do                            			\
		echo "-incdir "$${i} >> ${BLD_DIR}/searchPaths_build;    		\
	done
	${XRUN_CMD} -64bit -elaborate -libext .v -libext .sv -f ${BLD_DIR}/searchPaths_build -ALLOWREDEFINITION ${BLD_DIR}/config.v ${DUT_VLOG_ARGS} ${DUT_VERILOG_FLIST} ${DUT_XILINX_TOP_TB} -smartorder -reflib ${CADENCE_XIL_LIB}/unisims_ver -reflib ${CADENCE_XIL_LIB}/unimacro_ver  -reflib ${CADENCE_XIL_LIB}/secureip  -reflib ${CADENCE_XIL_LIB}/xpm -afile ${SIM_DIR}/pli/v2c.tab -sv_lib ${SIM_DIR}/lib/dll/libvpp.so  -dpiimpheader imp.h  -loadpli1 ${SIM_DIR}/lib/dll/libvpp.so:export -xmlibdirname ${BLD_DIR}/xcelium.d -log ${BLD_DIR}/presession.log  ${CADENCE_COV_COM_ARGS} ${SAHANLDER_FILE} -loadvpi ${BLD_DIR}/xcelium.d/run.d/librun.so:boot 
	touch $@

#
# NOTE: double :: rule!!
#
merge::
ifeq (${CADENCE},1)
	@if test ! -d ${CAD_TOP_COVERAGE}; then		\
		mkdir  ${CAD_TOP_COVERAGE};		\
	fi
	@if test ! -d ${CAD_TOP_COVERAGE}/${TEST_SUITE}; then	\
		mkdir  ${CAD_TOP_COVERAGE}/${TEST_SUITE};	\
	fi
	rm -rf ${CAD_TOP_COVERAGE}/${TEST_SUITE}/*
	mkdir  ${CAD_TOP_COVERAGE}/${TEST_SUITE}/merge
	@if test -d ${DUT_COVERAGE_PATH}/${TEST_SUITE}; then	\
		echo "Merging Cadene coverage files under ${DUT_COVERAGE_PATH} ..."; \
		${IMC_CMD}  -execcmd "merge ${DUT_COVERAGE_PATH}/${TEST_SUITE}/* -overwrite -message 1 -out ${DUT_COVERAGE_PATH} "; \
	fi 
	cp ${DUT_COVERAGE_PATH}/*.ucm ${CAD_TOP_COVERAGE}/${TEST_SUITE}/.
	cp ${DUT_COVERAGE_PATH}/*.ucd ${CAD_TOP_COVERAGE}/${TEST_SUITE}/merge/.
endif

#
# Cadence Stuffs
#
CAD_TOP_COVERAGE           ?= ${SIM_DIR}/cad_coverage
override DUT_COVERAGE_PATH  = ${BLD_DIR}/cad_coverage

mergeAll:: .force
ifeq (${CADENCE},1)
	@if test ! -d ${CAD_TOP_COVERAGE}; then	\
		mkdir  ${CAD_TOP_COVERAGE};		\
	fi
	rm -rf ${CAD_TOP_COVERAGE}/*
	touch ${CAD_TOP_COVERAGE}/runFile.txt
	@for i in ${TEST_GROUP}; do	\
		echo "./$${i}/*" >> ${CAD_TOP_COVERAGE}/runFile.txt; \
		(cd $${i}; make merge) 	\
	done	
	${IMC_CMD}  -execcmd "merge -runfile ${CAD_TOP_COVERAGE}/runFile.txt -initial_model union_all -message 1 -out ${CAD_TOP_COVERAGE}/merge;"
	${IMC_CMD}  -execcmd "load -run ${CAD_TOP_COVERAGE}/merge; report_metrics -detail -report_type regular -overwrite -all -out ${CAD_TOP_COVERAGE}/merge/detail_report " ; 
endif


