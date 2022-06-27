#--------------------------------------------------------------------------------------
# Copyright 2022 Massachusets Institute of Technology
# SPDX short identifier: BSD-2-Clause
#
# File Name:      cep_buildSW.make
# Program:        Common Evaluation Platform (CEP)
# Description:    Co-Simulation makefile for CEP Software
# Notes:          This file cannot be invoked by itself
#
#--------------------------------------------------------------------------------------

# If operating in Bare Metal mode, pass the RISCV wrapper name to the compiler
ifeq "${DUT_SIM_MODE}" "BARE_MODE"
RISCV_WRAPPER           = ./riscv_wrapper.img
COMMON_CFLAGS	        += -DRISCV_WRAPPER=\"${RISCV_WRAPPER}\"
endif

ifeq (${NOWAVE},1)
COMMON_CFLAGS	        += -DNOWAVE
RISCV_BARE_CFLAGS       += -DNOWAVE
endif

ifeq (${ASIC_MODE},1)
COMMON_CFLAGS	        += -DASICMODE
RISCV_BARE_CFLAGS       += -DASICMODE
endif

ifeq (${FPGA_SW_BUILD},1)
RISCV_BARE_CFLAGS       += -DFPGASWBUILD
endif

#--------------------------------------------------------------------------------------
# Create lists of libraries, sources, and object files
#
# Object file extensions:
#   *.o    = object for c-side
#   *.obj  = object for sim-side (file to be shared with PLI)
#   *.bobj = object file for RISCV's bare metal
#
#--------------------------------------------------------------------------------------
SRC_D           = ${COSIM_TOP_DIR}/src
APIS_D          = ${DRIVERS_DIR}/cep_tests
DIAG_D          = ${DRIVERS_DIR}/diag
VECTOR_D		= ${DRIVERS_DIR}/vectors
SHARE_D         = ${COSIM_TOP_DIR}/share
SIMDIAG_D       = ${COSIM_TOP_DIR}/simDiag
PLI_D           = ${COSIM_TOP_DIR}/pli
BARE_D          = ${DRIVERS_DIR}/bare

SRC_LIB_DIR     = ${LIB_DIR}/src
APIS_LIB_DIR    = ${LIB_DIR}/cep_tests
DIAG_LIB_DIR    = ${LIB_DIR}/diag
SHARE_LIB_DIR   = ${LIB_DIR}/share
SIMDIAG_LIB_DIR = ${LIB_DIR}/simDiag
PLI_LIB_DIR     = ${LIB_DIR}/pli
BARE_LIB_DIR    = ${LIB_DIR}/bare

SRC_SRC         = $(wildcard ${SRC_D}/*.cc)
APIS_SRC        = $(wildcard ${APIS_D}/*.cc)
DIAG_SRC        = $(wildcard ${DIAG_D}/*.cc)
SHARE_SRC       = $(wildcard ${SHARE_D}/*.cc)
SIMDIAG_SRC     = $(wildcard ${SIMDIAG_D}/*.cc)
PLI_SRC         = $(wildcard ${PLI_D}/*.cc)

# Bare metal source includes both .c and .S files
BARE_SRC        = $(wildcard ${BARE_D}/*.c)
BARE_SRC        += $(wildcard ${BARE_D}/*.S)

SRC_H           = $(wildcard ${SRC_D}/*.h)
APIS_H          = $(wildcard ${APIS_D}/*.h)
DIAG_H          = $(wildcard ${DIAG_D}/*.h)
SHARE_H         = $(wildcard ${SHARE_D}/*.h)
SIMDIAG_H       = $(wildcard ${SIMDIAG_D}/*.h)
PLI_H           = $(wildcard ${PLI_D}/*.h)
BARE_H          = $(wildcard ${BARE_D}/*.h)

# Create list of C/H files
ALL_C_FILES      = \
	${SRC_H}         ${SRC_SRC}         \
	${APIS_H}        ${APIS_SRC}        \
	${DIAG_H}        ${DIAG_SRC}        \
	${SHARE_H}       ${SHARE_SRC}       \
	${SIMDIAG_H}     ${SIMDIAG_SRC}     \
	${PLI_H}         ${PLI_SRC}         \
	${BARE_H}        ${BARE_SRC}        

# list of objects: 
SRC_O_LIST        = $(foreach t,${notdir $(subst .cc,.o,	${SRC_SRC})},     		${SRC_LIB_DIR}/${t})
APIS_O_LIST       = $(foreach t,${notdir $(subst .cc,.o,	${APIS_SRC})},    		${APIS_LIB_DIR}/${t})
DIAG_O_LIST       = $(foreach t,${notdir $(subst .cc,.o,	${DIAG_SRC})},    		${DIAG_LIB_DIR}/${t})
SHARE_O_LIST      = $(foreach t,${notdir $(subst .cc,.o,	${SHARE_SRC})},   		${SHARE_LIB_DIR}/${t})
SIMDIAG_O_LIST    = $(foreach t,${notdir $(subst .cc,.o,	${SIMDIAG_SRC})}, 		${SIMDIAG_LIB_DIR}/${t})
PLI_O_LIST        = $(foreach t,${notdir $(subst .cc,.o,	${PLI_SRC})},     		${PLI_LIB_DIR}/${t})
BARE_O_LIST_1     = $(foreach t,${notdir $(subst .c,.o,		${BARE_SRC})},     		${BARE_LIB_DIR}/${t})
BARE_O_LIST       = $(foreach t,${notdir $(subst .S,.o,		${BARE_O_LIST_1})},    	${BARE_LIB_DIR}/${t})

SRC_OBJ_LIST      = $(subst .o,.obj,${SRC_O_LIST})
APIS_OBJ_LIST     = $(subst .o,.obj,${APIS_O_LIST})
DIAG_OBJ_LIST     = $(subst .o,.obj,${DIAG_O_LIST})
SHARE_OBJ_LIST    = $(subst .o,.obj,${SHARE_O_LIST})
SIMDIAG_OBJ_LIST  = $(subst .o,.obj,${SIMDIAG_O_LIST})
PLI_OBJ_LIST      = $(subst .o,.obj,${PLI_O_LIST})
BARE_OBJ_LIST     = $(subst .o,.obj,${BARE_O_LIST})

SRC_BOBJ_LIST     = $(subst .o,.bobj,${SRC_O_LIST})
APIS_BOBJ_LIST    = $(subst .o,.bobj,${APIS_O_LIST})
DIAG_BOBJ_LIST    = $(subst .o,.bobj,${DIAG_O_LIST})
SHARE_BOBJ_LIST   = $(subst .o,.bobj,${SHARE_O_LIST})
SIMDIAG_BOBJ_LIST = $(subst .o,.bobj,${SIMDIAG_O_LIST})
PLI_BOBJ_LIST     = $(subst .o,.bobj,${PLI_O_LIST})
BARE_BOBJ_LIST    = $(subst .o,.bobj,${BARE_O_LIST})

# Create directories if not there (check first via order-only-prerequisites)
$(SRC_O_LIST)     : | ${SRC_LIB_DIR}
$(APIS_O_LIST)    : | ${APIS_LIB_DIR}
$(DIAG_O_LIST)    : | ${DIAG_LIB_DIR}
$(SHARE_O_LIST)   : | ${SHARE_LIB_DIR}
$(SIMDIAG_O_LIST) : | ${SIMDIAG_LIB_DIR}
$(BARE_BOBJ_LIST) : | ${BARE_LIB_DIR}
$(PLI_OBJ_LIST)   : | ${PLI_LIB_DIR}

OBJECT_DIR_LIST  += ${SRC_LIB_DIR} ${APIS_LIB_DIR} ${DIAG_LIB_DIR} ${SHARE_LIB_DIR} ${SIMDIAG_LIB_DIR} ${BARE_LIB_DIR} ${PLI_LIB_DIR}
${OBJECT_DIR_LIST}:
	mkdir -p $@
#--------------------------------------------------------------------------------------



#--------------------------------------------------------------------------------------
# Auto-extract the MAJOR/MINOR version from scala
#--------------------------------------------------------------------------------------
CEP_ADR_SCALA_FILE = ${REPO_TOP_DIR}/generators/mitll-blocks/src/main/scala/cep_addresses.scala
CEP_VER_H_FILE     = ${DRIVERS_DIR}/cep_tests/cep_version.h

${CEP_VER_H_FILE} : ${CEP_ADR_SCALA_FILE}
	@echo "// auto-extracted from ${CEP_ADR_SCALA_FILE}" > ${CEP_VER_H_FILE}
	@echo "// Do not modify" >> ${CEP_VER_H_FILE}
	@echo "#ifndef CEP_VERSION_H" >> ${CEP_VER_H_FILE}
	@echo "#define CEP_VERSION_H" >> ${CEP_VER_H_FILE}
	@grep CEP_MAJOR_VERSION ${CEP_ADR_SCALA_FILE} | sed -e 's/val/#define/' -e 's/=//' >> ${CEP_VER_H_FILE}
	@grep CEP_MINOR_VERSION ${CEP_ADR_SCALA_FILE} | sed -e 's/val/#define/' -e 's/=//' >> ${CEP_VER_H_FILE}
	@echo "#endif" >> ${CEP_VER_H_FILE}
	touch $@
#--------------------------------------------------------------------------------------



#--------------------------------------------------------------------------------------
# Convert specified .incl (Verilog) files to .h which will be placed in the $SHARE_D
#--------------------------------------------------------------------------------------
V2C_INCL_FILE_LIST	:= 	${DVT_DIR}/v2c_cmds.incl \
						${DVT_DIR}/cep_adrMap.incl
V2C_H_FILE_LIST     :=  $(foreach t,${notdir $(subst .incl,.h,${V2C_INCL_FILE_LIST})}, ${SHARE_D}/${t})

# This rule will apply to any .h we try to build in the ${SHARE_D}.  Fortunately, the only .h
# that are build are those being converted from .incl
${SHARE_D}/%.h: ${DVT_DIR}/%.incl ${V2C_CMD}
	@${V2C_CMD} $< $@

build_v2c: ${V2C_H_FILE_LIST}
#-------------------------------------------------------------------------------------



#-------------------------------------------------------------------------------------
# Common variables
#-------------------------------------------------------------------------------------
# Create a common list of include directories.
ifeq (${MODELSIM}, 1)
COMMON_INCLUDE_DIR_LIST	:= 	${SRC_D} \
							${APIS_D} \
							${DIAG_D} \
							${SHARE_D} \
							${SIMDIAG_D} \
							${PLI_D} \
							${QUESTASIM_PATH}/../include 
else ifeq (${CADENCE}, 1)
COMMON_INCLUDE_DIR_LIST	:= 	${SRC_D} \
							${APIS_D} \
							${DIAG_D} \
							${SHARE_D} \
							${SIMDIAG_D} \
							${PLI_D} \
							${XCELIUM_INSTALL}/tools/include 
endif

COMMON_INCLUDE_LIST		:= $(foreach t,${COMMON_INCLUDE_DIR_LIST}, -I ${t})

# A common set of dependencies (for all test modes)
COMMON_DEPENDENCIES		:= ${V2C_H_FILE_LIST} ${CEP_VER_H_FILE} ${PERSUITE_CHECK} ${ALL_C_FILES}

# Variables related to the RISCV Toolset (RISCV must already be defined)
RISCV_GCC         		:= ${RISCV}/bin/riscv64-unknown-elf-gcc
RISCV_LD 				:= ${RISCV}/bin/riscv64-unknown-elf-ld
RISCV_OBJDUMP     		:= ${RISCV}/bin/riscv64-unknown-elf-objdump
RISCV_OBJCOPY 			:= ${RISCV}/bin/riscv64-unknown-elf-objcopy
RISCV_HEXDUMP     		= /usr/bin/hexdump
RISCV_AR          		:= ${RISCV}/bin/riscv64-unknown-elf-gcc-ar
RISCV_RANLIB      		:= ${RISCV}/bin/riscv64-unknown-elf-gcc-ranlib

# Flags related to RISCV Virtual tests
RISCV_VIRT_CFLAGS  		+= -march=rv64g -mabi=lp64 -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -DENTROPY=0xdb1550c -static 
RISCV_VIRT_LFLAGS  		+= -T${DRIVERS_DIR}/virtual/link.ld
RISCV_VIRT_CFILES  		+= ${DRIVERS_DIR}/virtual/*.c ${DRIVERS_DIR}/virtual/multi_entry.S 
RISCV_VIRT_INC     		+= -I${DRIVERS_DIR}/virtual -I${RISCV_TEST_DIR}/isa/macros/scalar 

# Flags related to RISCV Baremetal tests (compiler and linker flags, with the exception of defines, copied from chipyard testchipip and fpga bootrom directories)
#
# -DBARE_MODE
# -DRISCV_CPU           - Defines specific to the CEP test suite
# -mcmodel=medany       - Generate code for the medium-any code model.  The program and its statically defined symbols must be within any single 2 GiB address range. 
#                         Programs can be statically or dynamically linked.
# -O2                   - Optimization level 2
# -Wall                 - Enable all warnings
# -nostartfiles         - Do not use the standard system startup files when linking
# -fno-builtin-printf   - Disable the builtin printf function (cep has a custom printf mechanism for bare metal simulation)
# -fno-common           - Specifies that the compiler places uninitialized global variables in the BSS section of the object file
# -march=rv64ima        - Specify supported instruction set
#                         RV64I - Base Integer Instruction Set v2.0
#                         M     - Standard Extension for Multiplication and Division v2.0
#                         A     - Standard Extension for Atomic Instructions v2.0
# -mabi=lp64            - Specify integer and floating poid calling convention
# -I                    - Add the directory dir to the list of directories to be searched for header files during preprocessing
# -static               - On systems that support dynamic linking, this overrides -pie and prevents linking with the shared libraries. On other systems, this option has no effect
# -nostdlib             - Do not use the standard system startup files or libraries when linking
# -T                    - Specify linker script
#
# -g                    - Produce debugging information in the operating system’s native format
# -c                    - Compile, but don't link files
# -lgcc                 - ?????
RISCV_BARE_CFLAGS  		+= -mcmodel=medany -O2 -Wall -fno-common -fno-builtin-printf 
RISCV_BARE_CFLAGS 		+= -I ${BARE_D} -I ${VECTOR_D} ${COMMON_INCLUDE_LIST}
RISCV_BARE_CFLAGS 		+= -mabi=lp64 -march=rv64ima
RISCV_BARE_CFLAGS 		+= -DBARE_MODE -DRISCV_CPU
RISCV_BARE_LFILE		+= ${BARE_D}/cep_link.lds
#RISCV_BARE_LFLAGS 		+= -static -nostdlib  -nostartfiles -lgcc -T ${RISCV_BARE_LFILE}
#RISCV_BARE_LFLAGS 		+= -L${LIB_DIR} -lriscv
RISCV_BARE_LFLAGS 		+= -static -nostdlib -nostartfiles -T ${RISCV_BARE_LFILE}

# Additional common flags
COMMON_CFLAGS			+= 	${COMMON_INCLUDE_LIST} \
							-DTL_CAPTURE=${TL_CAPTURE} \
							-DVECTOR_D=\"${VECTOR_D}\" \
							-g -std=gnu++11 \
							-Wno-format -Wno-narrowing \
							-DBIG_ENDIAN
COMMON_LDFLAGS        	=

# Flags for Hardware and Software simulation compilations
SIM_HW_CFLAGS			:= 	${COMMON_CFLAGS} -DSIM_ENV_ONLY -D_SIM_HW_ENV
SIM_SW_CFLAGS			:= 	${COMMON_CFLAGS} -DSIM_ENV_ONLY -D_SIM_SW_ENV 

# Switches to indicate what libraries are being used (not applicable to bare metal)
LIBRARY_SWITCHES  		= -lpthread -lcryptopp
#-------------------------------------------------------------------------------------



#--------------------------------------------------------------------------------------
# Rules to build object files.. based on the type and where they are going
#--------------------------------------------------------------------------------------
${SRC_LIB_DIR}/%.o ${SRC_LIB_DIR}/%.obj: ${SRC_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${APIS_LIB_DIR}/%.o ${APIS_LIB_DIR}/%.obj: ${APIS_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${BARE_LIB_DIR}/%.o ${BARE_LIB_DIR}/%.obj: ${BARE_D}/%.c ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${DIAG_LIB_DIR}/%.o ${DIAG_LIB_DIR}/%.obj: ${DIAG_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${SIMDIAG_LIB_DIR}/%.o ${SIMDIAG_LIB_DIR}/%.obj: ${SIMDIAG_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${PLI_LIB_DIR}/%.o ${PLI_LIB_DIR}/%.obj: ${PLI_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -fPIC -c -o $@ $< 

# .o & .obj  not same rule
${SHARE_LIB_DIR}/%.o: ${SHARE_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${SHARE_LIB_DIR}/%.obj: ${SHARE_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -fPIC -c -o $@ $< 

# .bobj for bare-metal
${SRC_LIB_DIR}/%.bobj: ${SRC_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@

${APIS_LIB_DIR}/%.bobj: ${APIS_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@

${BARE_LIB_DIR}/%.bobj: ${BARE_D}/%.S ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@

${BARE_LIB_DIR}/%.bobj: ${BARE_D}/%.c ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@

${DIAG_LIB_DIR}/%.bobj: ${DIAG_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@

${SIMDIAG_LIB_DIR}/%.bobj: ${SIMDIAG_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@

${SHARE_LIB_DIR}/%.bobj: ${SHARE_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@

${PLI_LIB_DIR}/%.bobj: ${PLI_D}/%.cc ${COMMON_DEPENDENCIES} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) -c $< -o $@
#--------------------------------------------------------------------------------------



# -----------------------------------------------------------------------
# If set, do not create a seperate ELF wrapper for bare metal mode
# (applicable for the ISA Tests)
# -----------------------------------------------------------------------
ifeq "$(findstring BUILTIN,${ELF_MODE})" "BUILTIN"
RISCV_WRAPPER_IMG = 
else
RISCV_WRAPPER_IMG = ${RISCV_WRAPPER}

# with -g, tests in virtual adr will run forever when it takes a page fault..!! (sending stuffs to console and stop)
# so build with -g for dump file only
ifeq (${VIRTUAL_MODE},1)
${RISCV_WRAPPER_IMG}: ${LIB_DIR}/.buildLibs ${RISCV_VIRT_CFILES} ${COMMON_DEPENDENCIES} riscv_virt.S riscv_wrapper.cc 
	$(RISCV_GCC) ${RISCV_VIRT_CFLAGS} ${RISCV_VIRT_LFLAGS} -g ${RISCV_VIRT_INC} $^ -o riscv_withG.elf
	${RISCV_OBJDUMP} -S -C -d -l -x riscv_withG.elf > riscv_wrapper.dump; rm riscv_withG.elf;
	$(RISCV_GCC) ${RISCV_VIRT_CFLAGS} ${RISCV_VIRT_LFLAGS} ${RISCV_VIRT_INC} $^ -o riscv_wrapper.elf
	${RISCV_HEXDUMP} -C riscv_wrapper.elf > riscv_wrapper.hex
	${RISCV_OBJCOPY} -O binary --change-addresses=-0x80000000 riscv_wrapper.elf riscv_wrapper.img
	${BIN_DIR}/createPassFail.pl riscv_wrapper.dump PassFail.hex
else
${RISCV_WRAPPER_IMG}: ${LIB_DIR}/.buildLibs ${RISCV_BARE_LFILE} ${COMMON_DEPENDENCIES} riscv_wrapper.cc 
	$(RISCV_GCC) $(RISCV_BARE_CFLAGS) ${RISCV_BARE_LFLAGS} riscv_wrapper.cc  ${RISCV_LIB} -o riscv_wrapper.elf
	${RISCV_OBJDUMP} -S -C -d -l -x riscv_wrapper.elf > riscv_wrapper.dump
	${RISCV_OBJCOPY} -O binary --change-addresses=-0x80000000 riscv_wrapper.elf riscv_wrapper.img
	${RISCV_HEXDUMP} -C riscv_wrapper.elf > riscv_wrapper.hex
endif

.PHONY: riscv_wrapper riscv_wrapper_sd_write
riscv_wrapper: ${RISCV_WRAPPER_IMG}

riscv_wrapper_sd_write: ${RISCV_WRAPPER_IMG}
ifneq (,$(wildcard ${DISK}))
	sudo dd if=$< of=$(DISK) bs=4096 conv=fsync
else
	$(error CEP_COSIM: Invalid DISK specified for SD write)
endif

endif

# -----------------------------------------------------------------------



# -----------------------------------------------------------------------
# Libray build targets
# -----------------------------------------------------------------------
# v2c_lib.a : src/cep_tests/diag/share
${V2C_LIB}: ${SRC_O_LIST} ${APIS_O_LIST} ${DIAG_O_LIST} ${SHARE_O_LIST} ${SIMDIAG_O_LIST}
	$(AR) rcuvs $@ $?
	touch $@

# libvpp.so : pli/share
${VPP_LIB}: ${SHARE_OBJ_LIST} ${PLI_OBJ_LIST}
	$(GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -fPIC -shared -g -o ${VPP_LIB} $?
	touch $@

# riscv_lib.a: bare/apis/diag
${RISCV_LIB}: ${APIS_BOBJ_LIST} ${DIAG_BOBJ_LIST} ${BARE_BOBJ_LIST}
	$(RISCV_AR) rcuvs $@ $?
	touch $@

# Library build target
${LIB_DIR}/.buildLibs: ${OBJECT_DIR_LIST} ${V2C_LIB} ${VPP_LIB} ${RISCV_LIB}
	touch $@

.PHONY: buildLibs
buildLibs: ${LIB_DIR}/.buildLibs
# -----------------------------------------------------------------------



# -----------------------------------------------------------------------
# Build Local Test
# -----------------------------------------------------------------------
LOCAL_CC_FILES  		:= $(wildcard ./*.cc)
LOCAL_H_FILES   		+= $(wildcard ./*.h)
LOCAL_OBJ_FILES 		+= $(LOCAL_CC_FILES:%.cc=%.o)

c_dispatch: ${LIB_DIR}/.buildLibs $(LOCAL_OBJ_FILES) ${COMMON_DEPENDENCIES}
	$(GCC) $(SIM_SW_CFLAGS) $(COMMON_LDFLAGS) -o $@ $(LOCAL_OBJ_FILES) ${V2C_LIB} ${LIBRARY_SWITCHES}

%.o: %.cc ${LIB_DIR}/.buildLibs ${LOCAL_H_FILES} ${COMMON_DEPENDENCIES} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<
# -----------------------------------------------------------------------



# -----------------------------------------------------------------------
# Clean Targets
# -----------------------------------------------------------------------
cleanLibs::
	-rm -rf ${LIB_DIR}/*
	-rm -rf ${LIB_DIR}/.buildLibs
# -----------------------------------------------------------------------
