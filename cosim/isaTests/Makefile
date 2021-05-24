#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************
#
# Can be override
#
STAND_ALONE = 1

all: runAll

#
# needed by common.make
#
COSIM_NAME  = $(shell cd ..; basename `pwd`)
DUT_TOP_DIR = $(shell cd ../..; pwd | ./${COSIM_NAME}/bin/strip_net.pl )
BLD_DIR     = $(shell cd .; pwd | ../bin/strip_net.pl )
TEST_SUITE  = $(shell basename ${BLD_DIR})
SIM_DIR     = ${DUT_TOP_DIR}/${COSIM_NAME}


# Always in regression mode
override REGRESS        = 1
override DUT_SIM_MODE	= BARE
#
# Top target!!!
#

#
# Either run one at a time or batch the wholething if supported
#


#
# override anything here before calling the common file
#
include ../common.make

#
# ------------------------------------
# Virtual-Mode test (single thread ONLY)
# Dont need to use all of them.. Just the pne has more page faults
# ------------------------------------
#
# pick one per group
ISA_VM_1CORE_ONLY_LIST= \
	rv64uc-v-rvc		\
	rv64ui-v-srliw		\
	rv64um-v-remw		\
	rv64ua-v-amoswap_d	\
	rv64uf-v-recoding	\
	rv64ud-v-move 		\

# make 4 cores
ISA_VM_LIST = $(foreach t,${ISA_VM_1CORE_ONLY_LIST},${t}0) 

#
#	$(foreach t,${ISA_VM_1CORE_ONLY_LIST},${t}1) \
#	$(foreach t,${ISA_VM_1CORE_ONLY_LIST},${t}2) \
#	$(foreach t,${ISA_VM_1CORE_ONLY_LIST},${t}3) 


#
# special tests that going toHost = Pass
#	rv64si-p-icache-alias \
#
ISA_SPECIAL_TESTS= \
	rv64mi-p-scall \
	rv64si-p-wfi \
	rv64ui-p-simple \
	rv64ud-p-structural \

#
# ------------------------------------
# single coree0 ONLY
# ------------------------------------
#
ISA_1CORE_ONLY_LIST = \
	rv64mi-p-mcsr \
	rv64si-p-dirty \
	rv64ua-p-lrsc \
	rv64uc-p-rvc \
	rv64ui-p-sb \
	rv64ui-p-sd \
	rv64ui-p-sh \
	rv64ui-p-sw \

#rv64um-v-remw \

#
# ------------------------------------
# Singel Thread Test (one core at a time)
# ------------------------------------
#
ISA_SINGETHR_LIST = \
	rv64ua-p-amoand_d \
	rv64ua-p-amoor_d \
	rv64ua-p-amoor_w \
	rv64ua-p-amoswap_d \
	rv64ua-p-amoswap_w \
	rv64ua-p-amoxor_d \
	rv64ua-p-amoxor_w \
	rv64ua-p-amoadd_d \
	rv64ua-p-amoadd_w \
	rv64ua-p-amoand_w \
	rv64ua-p-amomax_d \
	rv64ua-p-amomaxu_d \
	rv64ua-p-amomaxu_w \
	rv64ua-p-amomax_w \
	rv64ua-p-amomin_d \
	rv64ua-p-amominu_d \
	rv64ua-p-amominu_w \
	rv64ua-p-amomin_w \
	rv64ud-p-ldst \

#
# ------------------------------------
# All cores can run in parallel
# ------------------------------------
#
ISA_PARALLEL_TEST_LIST = \
	rv64uf-p-move \
	rv64mi-p-access \
	rv64mi-p-breakpoint \
	rv64mi-p-csr \
	rv64mi-p-illegal \
	rv64mi-p-ma_addr \
	rv64mi-p-ma_fetch \
	rv64mi-p-sbreak \
	rv64si-p-csr \
	rv64si-p-ma_fetch \
	rv64si-p-sbreak \
	rv64si-p-scall \
	rv64ud-p-fadd \
	rv64ud-p-fclass \
	rv64ud-p-fcmp \
	rv64ud-p-fcvt \
	rv64ud-p-fcvt_w \
	rv64ud-p-fdiv \
	rv64ud-p-fmadd \
	rv64ud-p-fmin \
	rv64ud-p-move \
	rv64ud-p-recoding \
	rv64uf-p-fadd \
	rv64uf-p-fclass \
	rv64uf-p-fcmp \
	rv64uf-p-fcvt \
	rv64uf-p-fcvt_w \
	rv64uf-p-fdiv \
	rv64uf-p-fmadd \
	rv64uf-p-fmin \
	rv64uf-p-ldst \
	rv64uf-p-recoding \
	rv64ui-p-add \
	rv64ui-p-addi \
	rv64ui-p-addiw \
	rv64ui-p-addw \
	rv64ui-p-and \
	rv64ui-p-andi \
	rv64ui-p-auipc \
	rv64ui-p-beq \
	rv64ui-p-bge \
	rv64ui-p-bgeu \
	rv64ui-p-blt \
	rv64ui-p-bltu \
	rv64ui-p-bne \
	rv64ui-p-fence_i \
	rv64ui-p-jal \
	rv64ui-p-jalr \
	rv64ui-p-lb \
	rv64ui-p-lbu \
	rv64ui-p-ld \
	rv64ui-p-lh \
	rv64ui-p-lhu \
	rv64ui-p-lui \
	rv64ui-p-lw \
	rv64ui-p-lwu \
	rv64ui-p-or \
	rv64ui-p-ori \
	rv64ui-p-sll \
	rv64ui-p-slli \
	rv64ui-p-slliw \
	rv64ui-p-sllw \
	rv64ui-p-slt \
	rv64ui-p-slti \
	rv64ui-p-sltiu \
	rv64ui-p-sltu \
	rv64ui-p-sra \
	rv64ui-p-srai \
	rv64ui-p-sraiw \
	rv64ui-p-sraw \
	rv64ui-p-srl \
	rv64ui-p-srli \
	rv64ui-p-srliw \
	rv64ui-p-srlw \
	rv64ui-p-sub \
	rv64ui-p-subw \
	rv64ui-p-xor \
	rv64ui-p-xori \
	rv64um-p-div \
	rv64um-p-divu \
	rv64um-p-divuw \
	rv64um-p-divw \
	rv64um-p-mul \
	rv64um-p-mulh \
	rv64um-p-mulhsu \
	rv64um-p-mulhu \
	rv64um-p-mulw \
	rv64um-p-rem \
	rv64um-p-remu \
	rv64um-p-remuw \
	rv64um-p-remw \


#
#
#
#
ISA_TEST_LIST = \
	${ISA_SPECIAL_TESTS}		\
	${ISA_VM_LIST}			\
	${ISA_1CORE_ONLY_LIST} 		\
	${ISA_SINGETHR_LIST} 		\
	${ISA_PARALLEL_TEST_LIST} 	\

#
# Special test, NOT ISA
#
#
# This test needs *openocd* to control the JTAG port
#
OPENOCD_CHECK := $(filter Debugger, $(shell openocd --version 2>&1 | grep Debugger))

ifeq "$(OPENOCD_CHECK)" "Debugger"
ISA_TEST_LIST += dtmTest
endif

TESTINFO_LIST := $(sort $(foreach t,${TALUS_TEST_LIST},${BLD_DIR}/${t}/testInfo.txt))

#
# talus tests use lot of memory => need big machines
#
runAll: ${BLD_DIR}/.ISACreated
ifeq (${STAND_ALONE},1)
	make cleanAll
	make libsBuild
endif
	-rm -rf ${BLD_DIR}/*_work ${BLD_DIR}/.*work_dependList.make
	-rm -rf ${BLD_DIR}/.*_dependList* ${BLD_DIR}/.is_checked
	make buildSim
	@for i in ${ISA_TEST_LIST}; do							\
		echo "    PEND : ${BLD_DIR}/$${i}"  > ${BLD_DIR}/$${i}/status;		\
		(cd ${BLD_DIR}/$${i}; rm -f *.o*; 					\
		make NOWAVE=1 TEST_NAME=$${i});						\
	done

summary:
	@date > regressionSummary
	@for i in ${ISA_TEST_LIST}; do				\
		cat ${BLD_DIR}/$${i}/status | tee -a regressionSummary;	\
	done


#
# ----------------------------
#
# ----------------------------
#
createISATests: .force
	@for i in ${ISA_VM_1CORE_ONLY_LIST}; do				\
		echo "== Generating VM 1CORE test directory for $${i}";		\
		make TEST_NAME=$${i} VIRTUAL_MODE=1 SINGLE_CORE_ONLY=1 SFX=0 makeTest; \
		make TEST_NAME=$${i} VIRTUAL_MODE=1 SINGLE_CORE_ONLY=2 SFX=1 makeTest; \
		make TEST_NAME=$${i} VIRTUAL_MODE=1 SINGLE_CORE_ONLY=4 SFX=2 makeTest; \
		make TEST_NAME=$${i} VIRTUAL_MODE=1 SINGLE_CORE_ONLY=8 SFX=3 makeTest; \
	done
	@for i in ${ISA_VM_SINGETHR_LIST}; do				\
		echo "== Generating VM SINGETHR test directory for $${i}";		\
		make TEST_NAME=$${i} VIRTUAL_MODE=1 SINGLE_THREAD=1 makeTest; \
	done
	@for i in ${ISA_1CORE_ONLY_LIST}; do				\
		echo "== Generating 1CORE_ONLY test directory for $${i}";		\
		make TEST_NAME=$${i} SINGLE_CORE_ONLY=1 makeTest; \
	done
	@for i in ${ISA_SINGETHR_LIST}; do				\
		echo "== Generating SINGETHR test directory for $${i}";		\
		make TEST_NAME=$${i} SINGLE_THREAD=1 makeTest; \
	done
	@for i in ${ISA_PARALLEL_TEST_LIST}; do				\
		echo "== Generating PARALLEL test directory for $${i}";		\
		make TEST_NAME=$${i} makeTest; \
	done
	@for i in ${ISA_SPECIAL_TESTS}; do				\
		echo "== Generating SPECIAL TESTs directory for $${i}";		\
		make TEST_NAME=$${i} PASS_IS_TO_HOST=1 makeTest; \
	done
	touch ${BLD_DIR}/.ISACreated


removeISATests: .force
	@rm -rf rv64*
	@rm -f ${BLD_DIR}/.ISACreated


isaTests: removeISATests createISATests
	@echo "done"

${BLD_DIR}/.ISACreated:
	@echo "*****************************************************"
	@echo "--- Please run 'make createISATests' to continue ----"
	@echo "*****************************************************"
	@exit 1
