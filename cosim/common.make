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
# ============================================================================
# DUT = CEP FPGA
# ============================================================================
#
# Specify the target vendor: XILINX, TSMC, etc...
# (no quote)
#
DUT_VENDOR 	= XILINX
#
# Only pick 1!!!
#
MODELSIM        = 1
CADENCE 	= 0
VERILATOR 	= 0

#
# Only ONE
#
ifeq (${CADENCE},1)
MODELSIM        = 0
VERILATOR 	= 0
endif

#
# BFM, BARE_METAL or LINUX
# (no quote)
#
DUT_SIM_MODE	= BFM
DUT_IN_VIRTUAL  = 0
#
# Some control flags
#
NOBUILD			= 0
NOWAVE          = 0
PROFILE         = 0
COVERAGE        = 0
USE_DPI         = 1
# transaction-level capturing
TL_CAPTURE      = 0
# cycle-by-cycle capturing
C2C_CAPTURE     = 0

#
#
# Tools
# can be override with env. variables
#
VIVADO_PATH	?= /opt/Xilinx/Vivado/2018.3
SIMULATOR_PATH	?= /opt/questa-2019.1/questasim/bin

#
# derived paths
#
SIM_DIR         ?= ${DUT_TOP_DIR}/cosim


# stupid VHDL package uses work.* inside so have no choice but to use work!!
WORK_NAME       = ${TEST_SUITE}_work
WORK_DIR        = ${BLD_DIR}/${WORK_NAME}

#
# export for modelsim
#
export DUT_SIM_DIR := ${SIM_DIR}

#
# more paths for C build
#
SHARE_DIR	= ${SIM_DIR}/share
PLI_DIR		= ${SIM_DIR}/pli
SIMDIAG_DIR	= ${SIM_DIR}/simDiag
SRC_DIR		= ${SIM_DIR}/src
LIB_DIR		= ${SIM_DIR}/lib
BIN_DIR         = ${SIM_DIR}/bin
DVT_DIR         = ${SIM_DIR}/dvt
INC_DIR         = ${SIM_DIR}/include
BHV_DIR         = ${DVT_DIR}/behav_models
DIAG_DIR        = ${LIB_DIR}/diag
DLL_DIR        	= ${LIB_DIR}/dll
VENDOR_DIR      = ${DVT_DIR}/vendor_models

#
# -----------------------------------------------------------------------
# should not touch anything below unless you know what you are doing!!!
# -----------------------------------------------------------------------
#
# Commands
#
VLOG_CMD	= ${SIMULATOR_PATH}/vlog
VCOM_CMD	= ${SIMULATOR_PATH}/vcom
oSCOM_CMD	= ${SIMULATOR_PATH}/scom
VOPT_CMD	= ${SIMULATOR_PATH}/vopt
VSIM_CMD        = ${SIMULATOR_PATH}/vsim
VLIB_CMD	= ${SIMULATOR_PATH}/vlib
VMAP_CMD	= ${SIMULATOR_PATH}/vmap
VMAKE_CMD	= ${SIMULATOR_PATH}/vmake
VCOVER_CMD      = ${SIMULATOR_PATH}/vcover

#
# C/C+ tools
#
GCC      	= /usr/bin/g++
#GCC      	= /usr/bin/gcc
AR       	= /usr/bin/ar
RANLIB   	= /usr/bin/ranlib
share_GCC      	= ${GCC}
RM       	= rm -f
LD       	= ${GCC}
LINKER       	= ${GCC}
VPP_CMD	        = ${BIN_DIR}/vpp.pl
MKDEPEND        = ${BIN_DIR}/mkDepend.pl

#
# Some variables 
#
ERROR_MESSAGE   = "OK"
CHECK_FLAG      = ${BLD_DIR}/.is_checked
CUR_CONFIG	= .CONFIG_${DUT_VENDOR}_${DUT_SIM_MODE}
SIM_DEPEND_TARGET = .${WORK_NAME}_dependList


#
# -------------------------------------------
# Some derived switches
# -------------------------------------------
#
DUT_VSIM_DO_FILE	= ${TEST_DIR}/vsim.do
#
ifeq (${NOWAVE},1)
COMMON_CFLAGS	        += -DNOWAVE
RISCV_BARE_CFLAG        += -DNOWAVE
endif

ifeq (${USE_DPI},1)
COMMON_CFLAGS	        += -DUSE_DPI
RISCV_BARE_CFLAG        += -DUSE_DPI
endif

#
# Auto-extract the MAJOR/MINOR version from scala
#
CEP_ADR_SCALA_FILE = ${DUT_TOP_DIR}/hdl_cores/freedom/mitll-blocks/src/main/scala/cep_addresses.scala
CEP_VER_H_FILE     = ${SIM_DIR}/drivers/cep_tests/cep_version.h

${CEP_VER_H_FILE} : ${CEP_ADR_SCALA_FILE}
	@echo "// auto-extracted from ${CEP_ADR_SCALA_FILE}" > ${CEP_VER_H_FILE}
	@echo "// Do not modify" >> ${CEP_VER_H_FILE}
	@echo "#ifndef CEP_VERSION_H" >> ${CEP_VER_H_FILE}
	@echo "#define CEP_VERSION_H" >> ${CEP_VER_H_FILE}
	@grep CEP_MAJOR_VERSION ${CEP_ADR_SCALA_FILE} | sed -e 's/val/#define/' -e 's/=//' >> ${CEP_VER_H_FILE}
	@grep CEP_MINOR_VERSION ${CEP_ADR_SCALA_FILE} | sed -e 's/val/#define/' -e 's/=//' >> ${CEP_VER_H_FILE}
	@echo "#endif" >> ${CEP_VER_H_FILE}
	touch $@

#
#
#
COMMON_CFLAGS	        += -DCAPTURE_CMD_SEQUENCE=${TL_CAPTURE}
COMMON_CFLAGS	        += -DC2C_CAPTURE=${C2C_CAPTURE}

#
# --------------
# Add more switches here is any
# --------------
#
ifeq (${PROFILE},1)
DUT_VSIM_ARGS += -autoprofile=${TEST_NAME}_profile
endif

DUT_COVERAGE_PATH = ${BLD_DIR}/coverage
ifeq (${COVERAGE},1)
#DUT_VSIM_ARGS     += -notoggleints -testname ${TEST_SUITE}_${TEST_NAME} -coverstore ${DUT_COVERAGE_PATH} -coverage
DUT_VSIM_ARGS     += -coverage 
DUT_VLOG_ARGS     += +cover=sbceft +define+COVERAGE 
DUT_VOPT_ARGS     += +cover=sbceft
endif

ifeq (${NOWAVE},1)
DUT_VLOG_ARGS     += +define+NOWAVE
endif

# Use our gcc instead of builtin form questa
DUT_VSIM_ARGS	  += -cpppath ${GCC}

#
# -------------------------------------------
# build the DUT's HW for simulation here
# -------------------------------------------
#
include ${SIM_DIR}/cep_buildChips.make
#
ifeq (${CADENCE},1)
include ${SIM_DIR}/cadence.make
endif
ifeq (${VERILATOR},1)
include ${SIM_DIR}/verilator.make
endif
#
COMMON_CFLAGS	        += -DRISCV_WRAPPER=\"${RISCV_WRAPPER}\"
#
# run the simulation
#
vsimOnly: ${BLD_DIR}/_info ${DLL_DIR}/libvpp.so ${DUT_VSIM_DO_FILE}
	${VSIM_CMD} -work ${WORK_DIR} -tab ${PLI_DIR}/v2c.tab -pli ${DLL_DIR}/libvpp.so -do ${DUT_VSIM_DO_FILE} ${DUT_VSIM_ARGS} ${WORK_DIR}.${DUT_OPT_MODULE} -batch -logfile ${TEST_DIR}/${TEST_NAME}.log

PLUSARGS      = " "
RANDOMIZE     = 1
UPDATE_INFO   = 1
TEST_INFO     = testHistory.txt
USE_GDB       = 0

# add +myplus=0 for stupid plus_arg
VSIM_CMD_LINE = "${VSIM_CMD} -work ${WORK_DIR} -t 1ps -tab ${PLI_DIR}/v2c.tab -pli ${DLL_DIR}/libvpp.so -sv_lib ${DLL_DIR}/libvpp -do ${DUT_VSIM_DO_FILE} ${DUT_VSIM_ARGS} ${WORK_DIR}.${DUT_OPT_MODULE} -batch -logfile ${TEST_DIR}/${TEST_NAME}.log +myplus=0"


# depend list

.vrun_flag: ${BLD_DIR}/_info ${DLL_DIR}/libvpp.so ${DUT_VSIM_DO_FILE} c_dispatch ${RISCV_WRAPPER_ELF}
ifeq (${COVERAGE},1)
	@if test ! -d ${DUT_COVERAGE_PATH}; then	\
		mkdir  ${DUT_COVERAGE_PATH};		\
	fi
endif
	${VPP_CMD} ${TEST_DIR}/c_dispatch ${RANDOMIZE} ${UPDATE_INFO} ${TEST_INFO} ${USE_GDB} ${TEST_DIR}/${TEST_NAME}.log ${COVERAGE} \"${VSIM_CMD_LINE}\"


#
# Create default local vsim do file if not there: one with sim for interactive and one for regression
#

#
# vsim.do file body
#
define VSIM_DO_BODY
set NumericStdNoWarnings 1
set StdArithNoWarnings 1
#
# Procedure to turn on/off wave and continue
# they are called from inside Verilog
#
proc wave_on {} {
     echo "Enable logging";	
     log -ports -r /* ;
}

proc wave_off {} {
     echo "Stop logging"	
     nolog -all;
}
proc dump_coverage {} {
     echo "Dumping Coverage";
     coverage save ../coverage/${TEST_NAME}.ucdb
}
#
# just run
#
run -all;
#
# should never get here anyway!!
#
quit
endef

export VSIM_DO_BODY

${TEST_DIR}/vsim.do:
	@if test ! -f $@; then	\
	echo "$$VSIM_DO_BODY" > $@; \
	fi

#
# -------------------------------------------
# To detect if any important flag has changed since last run
# -------------------------------------------
#
PERSUITE_CHECK = ${BLD_DIR}/.PERSUITE_${DUT_SIM_MODE}_${NOWAVE}_${COVERAGE}_${PROFILE}

#
# delete other mode marker if just switch
# always do this check
${PERSUITE_CHECK}: 
	if test ! -f ${PERSUITE_CHECK}; then rm -f ${BLD_DIR}/.PERSUITE_*; touch ${PERSUITE_CHECK}; fi

#
# ====================================
# Convert verilog to "C"
# ====================================
#
V2C_FILE_LIST       := 	v2c_cmds.h 	\
			cep_adrMap.h

VERILOG_DEFINE_LIST := $(foreach t,${V2C_FILE_LIST}, ${INC_DIR}/${t})
VERILOG_DEFINE_LIST += ${PERSUITE_CHECK}
VERILOG_DEFINE_LIST += ${CEP_VER_H_FILE}


build_v2c: ${VERILOG_DEFINE_LIST}

${INC_DIR}/v2c_cmds.h : ${DVT_DIR}/v2c_cmds.incl ${BIN_DIR}/v2c.pl 
	${BIN_DIR}/v2c.pl $< $@

${INC_DIR}/cep_adrMap.h : ${DVT_DIR}/cep_adrMap.incl ${BIN_DIR}/v2c.pl 
	${BIN_DIR}/v2c.pl $< $@



#
# =======================================
# Driver stuffs
# =======================================
#
DRIVER_DIR_LIST := ${SIM_DIR}/drivers/diag	\
		${SIM_DIR}/drivers/cep_tests    \
		${SIM_DIR}/drivers/vectors 

#
# -------------------------------------------
# C/C++ stuffs
# -------------------------------------------
#
DRIVER_GCC                  := $(GCC)
DRIVER_INC_LIST             := $(foreach t,${DRIVER_DIR_LIST}, -I ${t})
DRIVER_INC_LIST             += -I ${SIM_DIR}/include
DRIVER_SOURCE_LIST          := $(foreach t,${DRIVER_DIR_LIST}, $(wildcard ${t}/*.cc))
DRIVER_OBJECTS_LIST         := $(subst .cc,.obj,${DRIVER_SOURCE_LIST})
SIMDIAG_DRIVER_OBJECTS      := $(foreach t,${notdir ${DRIVER_OBJECTS_LIST}}, ${DIAG_DIR}/${t})

COMMON_CFLAGS	+= -I ${PLI_DIR} -I ${INC_DIR} -I ${SHARE_DIR}  \
		-I ${SIMDIAG_DIR} $(DRIVER_INC_LIST)		\
		-g  -std=gnu++11 \
		-Wno-format -Wno-narrowing 

COMMON_CFLAGS	+= -DBIG_ENDIAN 

SIM_HW_CFLAGS	= ${COMMON_CFLAGS} -I ${SIMULATOR_PATH}/../include	\
		-D_SIM_HW_ENV -DSIM_ENV_ONLY

SIM_SW_CFLAGS	= ${COMMON_CFLAGS}	\
		-D_SIM_SW_ENV -DSIM_ENV_ONLY

LAB_CFLAGS	= ${COMMON_CFLAGS}	\


#
# *.obj is for build C/C++ stuffs for sim/lab
#
%.obj: %.cc ${VERILOG_DEFINE_LIST}
	$(DRIVER_GCC) $(SIM_SW_CFLAGS) -c -o $@ $<
	$(DRIVER_GCC) $(LAB_CFLAGS) -c -o ${DIAG_DIR}/$(notdir $@) $< 

# use by both C and PLI
SHARE_SOURCE          = $(wildcard ${SHARE_DIR}/*.cc)
SHARE_OBJECTS         = $(subst .cc,.o,${SHARE_SOURCE}) 
SHARE_VERILOG_OBJECTS = $(foreach t,${notdir ${SHARE_OBJECTS}}, ${DLL_DIR}/${t}) 
SHARE_DIAG_OBJECTS    = $(foreach t,${notdir ${SHARE_OBJECTS}}, ${DIAG_DIR}/${t})

${SHARE_DIR}/%.o: ${SHARE_DIR}/%.cc ${SHARE_DIR}/%.h ${VERILOG_DEFINE_LIST}
	$(GCC) $(SIM_SW_CFLAGS) -c -o $@ $< 
	$(GCC) $(LAB_CFLAGS) -c -o ${DIAG_DIR}/$(notdir $@) $< 
	$(GCC) $(SIM_HW_CFLAGS)  -DDLL_SIM -D_REENTRANT -fPIC -c -o ${DLL_DIR}/$(notdir $@) $< 

#
# PLI only
#
PLI_SOURCE  = $(wildcard ${PLI_DIR}/*.cc)
PLI_OBJ     = $(subst .cc,.o,${PLI_SOURCE})
PLI_OBJECTS = $(foreach t,${notdir ${PLI_OBJ}}, ${DLL_DIR}/${t})

${DLL_DIR}/%.o: ${PLI_DIR}/%.cc ${PLI_DIR}/%.h  ${VERILOG_DEFINE_LIST}
	$(GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -D_REENTRANT -fPIC -c -o $@ $< 

#
# Simulated Diag Object (portable anywhere)
#
SIMDIAG_SOURCE 		:= $(wildcard ${SIMDIAG_DIR}/*.cc)
SIMDIAG_OBJECTS 	:= $(subst .cc,.o,${SIMDIAG_SOURCE})
SIMDIAG_DIAG_OBJECTS    = $(foreach t,${notdir ${SIMDIAG_OBJECTS}}, ${DIAG_DIR}/${t}) 

${SIMDIAG_DIR}/%.o: ${SIMDIAG_DIR}/%.cc ${SIMDIAG_DIR}/%.h ${VERILOG_DEFINE_LIST}
	$(GCC) $(SIM_SW_CFLAGS) -c -o $@ $< 
	$(GCC) $(LAB_CFLAGS) -c -o ${DIAG_DIR}/$(notdir $@) $< 


#
# use by both now
#
C_SOURCE  = $(wildcard ${SRC_DIR}/*.cc)
C_OBJECTS = $(subst .cc,.o,${C_SOURCE})
C_DIAG_OBJECTS  = $(foreach t,${notdir ${C_OBJECTS}}, ${DIAG_DIR}/${t})
C_SHARE_OBJECTS = $(foreach t,${notdir ${C_OBJECTS}}, ${DLL_DIR}/${t})

${SRC_DIR}/%.o: ${SRC_DIR}/%.cc ${INC_DIR}/%.h ${VERILOG_DEFINE_LIST}
	$(GCC) $(SIM_SW_CFLAGS) -c -o $@ $< 
	$(GCC) $(LAB_CFLAGS) -c -o ${DIAG_DIR}/$(notdir $@) $< 

# building all AR
${LIB_DIR}/v2c_lib.a: ${LIB_DIR}/share_lib.a ${VERILOG_DEFINE_LIST} ${C_OBJECTS} ${DRIVER_OBJECTS_LIST} $(SIMDIAG_OBJECTS) 
ifeq (${NOBUILD},0)
	$(AR) crv $@ $(C_OBJECTS) ${DRIVER_OBJECTS_LIST} $(SHARE_OBJECTS) $(SIMDIAG_OBJECTS) 
	$(RANLIB) $@
endif

${LIB_DIR}/share_lib.a: $(SHARE_OBJECTS)
ifeq (${NOBUILD},0)
	$(AR) crv $@ $(SHARE_OBJECTS)
	$(RANLIB) $@
endif

# building diag lib
${LIB_DIR}/diag_lib.a: ${LIB_DIR}/v2c_lib.a
ifeq (${NOBUILD},0)
	$(AR) crv $@ $(C_DIAG_OBJECTS) $(SHARE_DIAG_OBJECTS) $(SIMDIAG_DIAG_OBJECTS) $(SIMDIAG_DRIVER_OBJECTS) 
	$(RANLIB) $@
endif

${DLL_DIR}/libvpp.so: ${LIB_DIR}/share_lib.a ${PLI_OBJECTS}
ifeq (${NOBUILD},0)
	$(share_GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -D_REENTRANT  -fPIC -shared  -g  \
		-o ${DLL_DIR}/libvpp.so ${SHARE_VERILOG_OBJECTS} ${PLI_OBJECTS}

endif	

comBuild: ${LIB_DIR}/share_lib.a ${PLI_OBJECTS}

libsBuild: ${LIB_DIR}/v2c_lib.a ${DLL_DIR}/libvpp.so

#
# to build local test
#
LOCAL_CC_FILES  := $(wildcard ./*.cc)
LOCAL_H_FILES   += $(wildcard ./*.h)
LOCAL_OBJ_FILES += $(LOCAL_CC_FILES:%.cc=%.o)

THREAD_SWITCH  = -lpthread -lcrypto -lssl
LDFLAGS        =

c_dispatch:  $(LOCAL_OBJ_FILES) ${LIB_DIR}/v2c_lib.a  ${VERILOG_DEFINE_LIST}
	$(GCC) $(SIM_SW_CFLAGS) $(LDFLAGS) -o $@ $(LOCAL_OBJ_FILES) ${LIB_DIR}/v2c_lib.a ${THREAD_SWITCH} 

%.o: %.cc ${LOCAL_H_FILES} ${LIB_DIR}/v2c_lib.a  ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<



#
# -------------------------------------------
# Just to make sure all things are OK to go ahead and "make"
# -------------------------------------------
#
ifeq "$(findstring OK,${ERROR_MESSAGE})" "OK"
${BLD_DIR}/.is_checked: 
	@echo "Checking for proper enviroment settings = ${ERROR_MESSAGE}"
	touch $@
else
${BLD_DIR}/.is_checked: .force
	@echo "ERROR: **** ${ERROR_MESSAGE} ****"
	@rm -rf ${BLD_DIR}/.is_checked
	@exit 1
endif
#
# -------------------------------------------
# Config change detect.. If so, force a rebuild
# -------------------------------------------
#
${BLD_DIR}/${CUR_CONFIG}:
	-rm -f  ${BLD_DIR}/.CONFIG_*
	touch $@


#
# -------------------------------------------
# ISA Auto test generation
# -------------------------------------------
#
#
# Individual test
#
makeTest: ${BLD_DIR}/${TEST_NAME}${SFX}/${TEST_NAME}${SFX}.dump
	@echo "Done"

# override at command line ONLY!!!
SINGLE_THREAD=0
VIRTUAL_MODE=0
PASS_IS_TO_HOST=0;

${BLD_DIR}/${TEST_NAME}${SFX}/${TEST_NAME}${SFX}.dump : ${RISCV_TEST_DIR}/isa/${TEST_NAME}
	@if test ! -d ${BLD_DIR}/${TEST_NAME}${SFX}; then	\
		mkdir  ${BLD_DIR}/${TEST_NAME}${SFX}; \
	fi
	@cp -f ${BLD_DIR}/testTemplate/Makefile        ${BLD_DIR}/${TEST_NAME}${SFX}/.
	@cp -f ${BLD_DIR}/testTemplate/*.h             ${BLD_DIR}/${TEST_NAME}${SFX}/.
	@cp -f ${BLD_DIR}/testTemplate/c_module.cc     ${BLD_DIR}/${TEST_NAME}${SFX}/.
	@rm -f ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
ifeq (${SINGLE_THREAD},1)
	@echo "#define SINGLE_THREAD_ONLY" >> ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
endif
ifneq (${SINGLE_CORE_ONLY},)
	@echo "#define SINGLE_CORE_ONLY ${SINGLE_CORE_ONLY}" >> ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
endif
ifeq (${VIRTUAL_MODE},1)
	@echo "#define MAX_TIMEOUT 200"    >> ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
	@echo "#define VIRTUAL_MODE"       >> ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
else
	@echo "#define MAX_TIMEOUT 200"    >> ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
endif
ifeq (${PASS_IS_TO_HOST},1)
	@echo "#define PASS_IS_TO_HOST"    >> ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
endif
	@cat ${BLD_DIR}/testTemplate/c_dispatch.cc >>        ${BLD_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
	@cp -f ${RISCV_TEST_DIR}/isa/${TEST_NAME}      ${BLD_DIR}/${TEST_NAME}${SFX}/riscv_wrapper.elf
	@cp -f ${RISCV_TEST_DIR}/isa/${TEST_NAME}.dump ${BLD_DIR}/${TEST_NAME}${SFX}/${TEST_NAME}${SFX}.dump
	@${BIN_DIR}/createPassFail.pl ${RISCV_TEST_DIR}/isa/${TEST_NAME}.dump ${BLD_DIR}/${TEST_NAME}${SFX}/PassFail.hex

#
# -------------------------------------------
# coverage for modelsim
# -------------------------------------------
# NOTE: double ::
#
merge::
ifeq (${MODELSIM},1)
	@if test -d ${DUT_COVERAGE_PATH}; then	\
		(cd ${DUT_COVERAGE_PATH}; ${VCOVER_CMD} merge -out ${TEST_SUITE}.ucdb ${DUT_COVERAGE_PATH}/*.ucdb;) \
	fi
endif

# use "coverage open <file>.ucdb" under vsim to open and view
MODELSIM_TOP_COVERAGE  = ${SIM_DIR}/coverage
MODELSIM_COV_DO_SCRIPT = ${SIM_DIR}/modelsim_coverage.do

mergeAll:: .force
ifeq (${MODELSIM},1)
	@if test ! -d ${MODELSIM_TOP_COVERAGE}; then	\
	   mkdir  ${MODELSIM_TOP_COVERAGE};		\
	fi
	@rm -rf ${MODELSIM_TOP_COVERAGE}/*
	@for i in ${TEST_GROUP}; do	\
	   (cd $${i}; make merge; cp coverage/$${i}.ucdb ${MODELSIM_TOP_COVERAGE}/.);	\
	done
	@(cd ${MODELSIM_TOP_COVERAGE};  ${VSIM_CMD} -64 -do ${MODELSIM_COV_DO_SCRIPT})
endif

#
# -------------------------------------------
# clean
# -------------------------------------------
#

clean: cleanAll

cleanAll:
	-rm -rf ${SIM_DIR}/*/*_work ${SIM_DIR}/*/.*work_dependList.make
	-rm -rf ${SIM_DIR}/*/*.o* ${SIM_DIR}/*/*/*.bo* ${SIM_DIR}/*/*/*.o*
	-rm -rf ${SIM_DIR}/*/*/status
	-rm -rf ${SIM_DIR}/bareMetalTests/*/*.elf ${SIM_DIR}/bareMetalTests/*/*.dump
	-rm -rf ${SIM_DIR}/*/.build*
	-rm -rf ${LIB_DIR}/*/*.o* ${LIB_DIR}/*.a ${LIB_DIR}/*/*.so
	-rm -rf ${SIM_DIR}/*/.*_dependList* ${SIM_DIR}/*/.is_checked
	-rm -rf ${SIM_DIR}/*/*/C2V*
	-rm -rf ${BHV_DIR}/VCShell*.v
	-rm -rf ${SIM_DIR}/*/xcelium.d ${SIM_DIR}/*/.cadenceBuild ${SIM_DIR}/*/*/cov_work ${SIM_DIR}/*/*/xrun.log
ifeq (${COVERAGE},1)
ifeq (${CADENCE},1)
	-rm -rf ${SIM_DIR}/*/cad_coverage
endif
ifeq (${MODELSIM},1)
	-rm -rf ${SIM_DIR}/*/coverage
	-rm -f ${SIM_DIR}/*/*/vsim.do
endif
endif
#
#
#
.force:

#
# -------------------------------------------
# Print usage
# -------------------------------------------
#
define MAKE_USAGE_HELP_BODY
make [NOWAVE=0|1] [COVERAGE=0|1] [PROFILE=0|1] [USE_GDB=0|1] <all|summary|cleanAll|merge|usage>

Options:
  NOWAVE=1      : turn off wave capturing. Default is ON for interactive. OFF for regression
  USE_GDB=1     : run the test under "gdb" debugger. Default is OFF. Should only be used for interactive session.
  COVERAGE=1    : run the test with coverage capture enable. Default is OFF
  PROFILE=1     : run the test with profiling turn on. Default is OFF
  TL_CAPTURE=1  : turn on Title-Link comand sequence capturing to be used for BARE Metal (and Unit-level testbench)
  C2C_CAPTURE=1 : turn on cycle-by-cycle at core's IO  for Unit testbench

Targets:
  all        : run the test. Default target (empty string = same as "all")
  summary    : print out regression summary (run at the top or suite directories.
  cleanAll   : clean up and force a rebuild of every thing
  merge      : merge coverage reports
  usage      : print this help lines

  see <common.make> for more options and targets
endef

export MAKE_USAGE_HELP_BODY

usage:
	-echo "$$MAKE_USAGE_HELP_BODY"


