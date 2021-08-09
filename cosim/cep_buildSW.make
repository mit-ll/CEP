#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      cep_buildSW.make
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Part of Makefile structure
#// Notes:          
#//
#//************************************************************************
#
# ---------------------------------
# Flags
# ---------------------------------
#
ifeq (${NOWAVE},1)
COMMON_CFLAGS	        += -DNOWAVE
RISCV_BARE_CFLAG        += -DNOWAVE
endif

ifeq (${USE_DPI},1)
COMMON_CFLAGS	        += -DUSE_DPI
RISCV_BARE_CFLAG        += -DUSE_DPI
endif

COMMON_CFLAGS	        += -DCAPTURE_CMD_SEQUENCE=${TL_CAPTURE}
COMMON_CFLAGS	        += -DC2C_CAPTURE=${C2C_CAPTURE}

# make
ifeq "$(findstring BFM,${DUT_SIM_MODE})" "BFM"
COMMON_CFLAGS	        += -DBFM_MODE
#
else ifeq "$(findstring BARE,${DUT_SIM_MODE})" "BARE"
#COMMON_CFLAGS	        += -DBARE_MODE
else
ERROR_MESSAGE   = "DUT_SIM_MODE=${DUT_SIM_MODE} is either not supported or please check spelling"
endif

DRIVER_DIR_LIST := ${SIM_DIR}/drivers/diag	\
		${SIM_DIR}/drivers/cep_tests    \
		${SIM_DIR}/drivers/vectors 
DRIVER_INC_LIST             := $(foreach t,${DRIVER_DIR_LIST}, -I ${t})
DRIVER_INC_LIST             += -I ${SIM_DIR}/include

COMMON_CFLAGS	        += -DRISCV_WRAPPER=\"${RISCV_WRAPPER}\"

COMMON_CFLAGS	+= -I ${PLI_DIR} -I ${INC_DIR} -I ${SHARE_DIR}  \
		-I ${SIMDIAG_DIR} $(DRIVER_INC_LIST)		\
		${EXTRA_COMMON_CFLAGS} \
		-g  -std=gnu++11 \
		-Wno-format -Wno-narrowing 

COMMON_CFLAGS	+= -DBIG_ENDIAN 

SIM_HW_CFLAGS	+= ${COMMON_CFLAGS} -I ${SIMULATOR_PATH}/../include	\
		-D_SIM_HW_ENV -DSIM_ENV_ONLY

SIM_SW_CFLAGS	= ${COMMON_CFLAGS}	\
		-D_SIM_SW_ENV -DSIM_ENV_ONLY


RISCV_BARE_CFLAG  += -DBARE_MODE -static -DRISCV_CPU
RISCV_BARE_CFLAG  += -mcmodel=medany -Wall -O2 -g -fno-common -nostdlib -fno-builtin-printf -I${BARE_SRC_DIR} $(DRIVER_INC_LIST) -I ${SIM_DIR}/share -I ${SIM_DIR}/include  ${EXTRA_COMMON_CFLAGS}
RISCV_BARE_LDFLAG += -static -nostdlib -nostartfiles -lgcc -DBARE_MODE 
RISCV_BARE_LDFILE  = ${BARE_SRC_DIR}/link.ld

THREAD_SWITCH  = -lpthread -lcryptopp
LDFLAGS        =


#
# --------------------------------
# LIBRARIES 
# --------------------------------
# name of libraries to be built
# XX_LIB_DIR can be override
#

#
# *.o    = object for c-side
# *.obj  = object for sim-side (file to be shared with PLI)
# *.bobj = object file for RISCV's bare metal
#
#
# List of source files
#
SRC_D           = ${SIM_DIR}/src
APIS_D          = ${SIM_DIR}/drivers/cep_tests
BARE_D          = ${SIM_DIR}/drivers/bare
DIAG_D          = ${SIM_DIR}/drivers/diag
SHARE_D         = ${SIM_DIR}/share
SIMDIAG_D       = ${SIM_DIR}/simDiag
PLI_D           = ${SIM_DIR}/pli
#
SRC_LIB_DIR     = ${XX_LIB_DIR}/src
APIS_LIB_DIR    = ${XX_LIB_DIR}/cep_tests
BARE_LIB_DIR    = ${XX_LIB_DIR}/bare
DIAG_LIB_DIR    = ${XX_LIB_DIR}/diag
SHARE_LIB_DIR   = ${XX_LIB_DIR}/share
SIMDIAG_LIB_DIR = ${XX_LIB_DIR}/simDiag
PLI_LIB_DIR     = ${XX_LIB_DIR}/pli

#
SRC_SRC         = $(wildcard ${SRC_D}/*.cc)
APIS_SRC        = $(wildcard ${APIS_D}/*.cc)
BARE_SRC        = $(wildcard ${BARE_D}/*.c)
DIAG_SRC        = $(wildcard ${DIAG_D}/*.cc)
SHARE_SRC       = $(wildcard ${SHARE_D}/*.cc)
SIMDIAG_SRC     = $(wildcard ${SIMDIAG_D}/*.cc)
PLI_SRC         = $(wildcard ${PLI_D}/*.cc)

SRC_H           = $(wildcard ${SRC_D}/*.h)
APIS_H          = $(wildcard ${APIS_D}/*.h)
BARE_H          = $(wildcard ${BARE_D}/*.h)
DIAG_H          = $(wildcard ${DIAG_D}/*.h)
SHARE_H         = $(wildcard ${SHARE_D}/*.h)
SIMDIAG_H       = $(wildcard ${SIMDIAG_D}/*.h)
PLI_H           = $(wildcard ${PLI_D}/*.h)
#
# Create list of C/H files
#
ALL_C_FILES      = \
	${SRC_H}         ${SRC_SRC}         \
	${APIS_H}        ${APIS_SRC}        \
	${BARE_H}        ${BARE_SRC}        \
	${DIAG_H}        ${DIAG_SRC}        \
	${SHARE_H}       ${SHARE_SRC}       \
	${SIMDIAG_H}     ${SIMDIAG_SRC}     \
	${PLI_H}         ${PLI_SRC}         

#
# list of objects: 
#
SRC_O_LIST        = $(foreach t,${notdir $(subst .cc,.o,${SRC_SRC})},     ${SRC_LIB_DIR}/${t})
APIS_O_LIST       = $(foreach t,${notdir $(subst .cc,.o,${APIS_SRC})},    ${APIS_LIB_DIR}/${t})
BARE_O_LIST       = $(foreach t,${notdir $(subst .c,.o,${BARE_SRC})},     ${BARE_LIB_DIR}/${t})
DIAG_O_LIST       = $(foreach t,${notdir $(subst .cc,.o,${DIAG_SRC})},    ${DIAG_LIB_DIR}/${t})
SHARE_O_LIST      = $(foreach t,${notdir $(subst .cc,.o,${SHARE_SRC})},   ${SHARE_LIB_DIR}/${t})
SIMDIAG_O_LIST    = $(foreach t,${notdir $(subst .cc,.o,${SIMDIAG_SRC})}, ${SIMDIAG_LIB_DIR}/${t})
PLI_O_LIST        = $(foreach t,${notdir $(subst .cc,.o,${PLI_SRC})},     ${PLI_LIB_DIR}/${t})

SRC_OBJ_LIST      = $(subst .o,.obj,${SRC_O_LIST})
APIS_OBJ_LIST     = $(subst .o,.obj,${APIS_O_LIST})
BARE_OBJ_LIST     = $(subst .o,.obj,${BARE_O_LIST})
DIAG_OBJ_LIST     = $(subst .o,.obj,${DIAG_O_LIST})
SHARE_OBJ_LIST    = $(subst .o,.obj,${SHARE_O_LIST})
SIMDIAG_OBJ_LIST  = $(subst .o,.obj,${SIMDIAG_O_LIST})
PLI_OBJ_LIST      = $(subst .o,.obj,${PLI_O_LIST})

SRC_BOBJ_LIST     = $(subst .o,.bobj,${SRC_O_LIST})
APIS_BOBJ_LIST    = $(subst .o,.bobj,${APIS_O_LIST})
BARE_BOBJ_LIST    = $(subst .o,.bobj,${BARE_O_LIST})
DIAG_BOBJ_LIST    = $(subst .o,.bobj,${DIAG_O_LIST})
SHARE_BOBJ_LIST   = $(subst .o,.bobj,${SHARE_O_LIST})
SIMDIAG_BOBJ_LIST = $(subst .o,.bobj,${SIMDIAG_O_LIST})
PLI_BOBJ_LIST     = $(subst .o,.bobj,${PLI_O_LIST})

#
# Create directories if not there (check first via order-only-prerequisites)
#
$(SRC_O_LIST)     : | ${SRC_LIB_DIR}
$(APIS_O_LIST)    : | ${APIS_LIB_DIR}
$(DIAG_O_LIST)    : | ${DIAG_LIB_DIR}
$(SHARE_O_LIST)   : | ${SHARE_LIB_DIR}
$(SIMDIAG_O_LIST) : | ${SIMDIAG_LIB_DIR}
$(BARE_BOBJ_LIST) : | ${BARE_LIB_DIR}
$(PLI_OBJ_LIST)   : | ${PLI_LIB_DIR}

OBJECT_DIR_LIST  += ${SRC_LIB_DIR} ${APIS_LIB_DIR} ${BARE_LIB_DIR} ${DIAG_LIB_DIR} ${SHARE_LIB_DIR} ${SIMDIAG_LIB_DIR} ${PLI_LIB_DIR}
${OBJECT_DIR_LIST}:
	mkdir $@

#
# ====================================
# Convert verilog to "C"
# ====================================
#
V2C_FILE_LIST       := 	v2c_cmds.h 	\
			cep_adrMap.h

#
# Auto-extract the MAJOR/MINOR version from scala
#
CEP_ADR_SCALA_FILE = ${XX_SIM_DIR}/../hdl_cores/freedom/mitll-blocks/src/main/scala/cep_addresses.scala
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

VERILOG_DEFINE_LIST := $(foreach t,${V2C_FILE_LIST}, ${INC_DIR}/${t})
VERILOG_DEFINE_LIST += ${PERSUITE_CHECK}
VERILOG_DEFINE_LIST += ${CEP_VER_H_FILE}
build_v2c: ${VERILOG_DEFINE_LIST}

${INC_DIR}/v2c_cmds.h : ${DVT_DIR}/v2c_cmds.incl ${BIN_DIR}/v2c.pl 
	${BIN_DIR}/v2c.pl $< $@

${INC_DIR}/cep_adrMap.h : ${DVT_DIR}/cep_adrMap.incl ${BIN_DIR}/v2c.pl 
	${BIN_DIR}/v2c.pl $< $@


#
# Rule to build object files
#
# .o & .obj: same rule
${SRC_LIB_DIR}/%.o ${SRC_LIB_DIR}/%.obj: ${SRC_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${APIS_LIB_DIR}/%.o ${APIS_LIB_DIR}/%.obj: ${APIS_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${BARE_LIB_DIR}/%.o ${BARE_LIB_DIR}/%.obj: ${BARE_D}/%.c ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${DIAG_LIB_DIR}/%.o ${DIAG_LIB_DIR}/%.obj: ${DIAG_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${SIMDIAG_LIB_DIR}/%.o ${SIMDIAG_LIB_DIR}/%.obj: ${SIMDIAG_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${PLI_LIB_DIR}/%.o ${PLI_LIB_DIR}/%.obj: ${PLI_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -D_REENTRANT -fPIC -c -o $@ $< 

# .o & .obj  not same rule
${SHARE_LIB_DIR}/%.o: ${SHARE_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_SW_CFLAGS) -I. -c -o $@ $<

${SHARE_LIB_DIR}/%.obj: ${SHARE_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -D_REENTRANT -fPIC -c -o $@ $< 

# .bobj for bare-metal
${SRC_LIB_DIR}/%.bobj: ${SRC_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

${APIS_LIB_DIR}/%.bobj: ${APIS_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

${BARE_LIB_DIR}/%.bobj: ${BARE_D}/%.c ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

${DIAG_LIB_DIR}/%.bobj: ${DIAG_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

${SIMDIAG_LIB_DIR}/%.bobj: ${SIMDIAG_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

${SHARE_LIB_DIR}/%.bobj: ${SHARE_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

${PLI_LIB_DIR}/%.bobj: ${PLI_D}/%.cc ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

# special S files
RISCV_BARE_CRTFILE = ${BARE_SRC_DIR}/crt.S

${BARE_LIB_DIR}/crt.bobj: ${RISCV_BARE_CRTFILE}
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

#
# -----------------------------------------------------------------------
# rules to build object file for bare metal under sim (bobj)
# -----------------------------------------------------------------------
#

ifeq ($(RE_USE_TEST),0)
riscv_wrapper.bobj: riscv_wrapper.cc
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@
endif

ifeq "$(findstring BUILTIN,${DUT_ELF_MODE})" "BUILTIN"
# blank it
undefine RISCV_WRAPPER_ELF
else

RISCV_WRAPPER_ELF = ${RISCV_WRAPPER}

#
# with -g, tests in virtual adr will run forever when it takes a page fault..!! (sending stuffs to console and stop)
# so build with -g for dump file only
#
#
ifeq (${DUT_IN_VIRTUAL},1)
riscv_wrapper.elf: riscv_virt.S riscv_wrapper.cc ${RISCV_VIRT_CFILES}
	$(RISCV_GCC) ${RISCV_VIRT_CFLAGS} ${RISCV_VIRT_LFLAGS} -g ${RISCV_VIRT_INC} $^ -o riscv_withG.elf
	${RISCV_OBJDUMP} -S -C -d -l -x riscv_withG.elf > riscv_wrapper.dump; rm riscv_withG.elf;
	$(RISCV_GCC) ${RISCV_VIRT_CFLAGS} ${RISCV_VIRT_LFLAGS} ${RISCV_VIRT_INC} $^ -o riscv_wrapper.elf
	${RISCV_HEXDUMP} -C riscv_wrapper.elf > riscv_wrapper.hex
	${BIN_DIR}/createPassFail.pl riscv_wrapper.dump PassFail.hex
else
riscv_wrapper.elf: riscv_wrapper.bobj ${RISCV_LIB}
	$(RISCV_GCC) -T ${RISCV_BARE_LDFILE} ${RISCV_BARE_LDFLAG} $^ -o $@
	${RISCV_OBJDUMP} -S -C -d -l -x riscv_wrapper.elf > riscv_wrapper.dump
	${RISCV_HEXDUMP} -C riscv_wrapper.elf > riscv_wrapper.hex

endif

endif

#
# v2c_lib.a : src/cep_tests/diag/share/simDiag
#
${V2C_LIB}: ${SRC_O_LIST} ${APIS_O_LIST} ${DIAG_O_LIST} ${SHARE_O_LIST} ${SIMDIAG_O_LIST}
ifeq (${NOBUILD},0)
	$(AR) crv $@ 					\
		$(shell ls ${SRC_LIB_DIR}/*.o) 		\
		$(shell ls ${APIS_LIB_DIR}/*.o) 	\
		$(shell ls ${DIAG_LIB_DIR}/*.o)		\
		$(shell ls ${SHARE_LIB_DIR}/*.o)	\
		$(shell ls ${SIMDIAG_LIB_DIR}/*.o)
	$(RANLIB) $@
endif

#
# libvpp.so : pli/share
#
${VPP_LIB}: ${SHARE_OBJ_LIST} ${PLI_OBJ_LIST}
ifeq (${NOBUILD},0)
	$(GCC) $(SIM_HW_CFLAGS) -DDLL_SIM -D_REENTRANT  -fPIC -shared  -g  \
		-o ${VPP_LIB}	\
		$(shell ls ${SHARE_LIB_DIR}/*.obj) \
		$(shell ls ${PLI_LIB_DIR}/*.obj) 
endif	

#
# riscv_lib.a: bare/apis/diag
#
${RISCV_LIB}: ${APIS_BOBJ_LIST} ${DIAG_BOBJ_LIST} ${BARE_BOBJ_LIST} ${BARE_LIB_DIR}/crt.bobj
ifeq (${NOBUILD},0)
	$(RISCV_AR) crv $@ \
		$(shell ls ${APIS_LIB_DIR}/*.bobj) \
		$(shell ls ${DIAG_LIB_DIR}/*.bobj) \
		$(shell ls ${BARE_LIB_DIR}/*.bobj) 
	$(RISCV_RANLIB) $@
endif	

#
# target
#
AUTO_C_DEPEND_FILE ?= ${SIM_DIR}/.c_children.mk

buildLibs : ${XX_LIB_DIR}/.buildLibs

# create list of files
${XX_LIB_DIR}/.buildLibs: ${RE_USE_TARGET} ${V2C_LIB} ${VPP_LIB} ${RISCV_LIB}
	-rm -f ${AUTO_C_DEPEND_FILE}
	@echo "ALL_C_FILES = \\" > ${SIM_DIR}/.c_children.mk
	@for i in ${ALL_C_FILES}; do				\
		echo "	$${i} \\" >> ${AUTO_C_DEPEND_FILE}; 	\
	done
	@echo "" >> ${AUTO_C_DEPEND_FILE}
	touch $@

#
# to build local test
#
LOCAL_CC_FILES  := $(wildcard ./*.cc)
LOCAL_H_FILES   += $(wildcard ./*.h)
LOCAL_OBJ_FILES += $(LOCAL_CC_FILES:%.cc=%.o)


ifeq ($(RE_USE_TEST),0) 
c_dispatch:  $(LOCAL_OBJ_FILES) ${V2C_LIB}  ${VERILOG_DEFINE_LIST}
	$(GCC) $(SIM_SW_CFLAGS) $(LDFLAGS) -o $@ $(LOCAL_OBJ_FILES) ${V2C_LIB} ${EXTRA_V2C_LIB} ${THREAD_SWITCH} 

endif

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
# Clean up
#
cleanLibs::
	-rm -rf ${V2C_LIB} ${VPP_LIB} ${RISCV_LIB} ${AUTO_C_DEPEND_FILE}
	-rm -rf ${XX_LIB_DIR}/*/*.o ${XX_LIB_DIR}/*/*.obj ${XX_LIB_DIR}/*/*.bobj

