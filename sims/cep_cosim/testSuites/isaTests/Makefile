#************************************************************************
# Copyright 2022 Massachusets Institute of Technology
#
# File Name:      Makefile
# Program:        Common Evaluation Platform (CEP)
# Description:    Makefile for isaTests test suite
# Notes:          Depending on the test category, the following
#                 defines are added to a particular tests c_dispatch.cc:
#					MAX_TIMEOUT 		- Passes the timeout parameter
#                     to the thread via the seed parameter.  Used both
#                     in checking if the program is loaded and if it has
#                     completed execution.
#                   SINGLE_CORE_ONLY	- 
#					SINGLE_THREAD_ONLY	-
#					PASS_WRITETOHOST	- Somes tests are considered
#                     passed when they reach the write_tohost function
#					VIRTUAL_MODE		- Affects both compilation and
#					  runtime behavior.
#
#************************************************************************

# Without the following, RHEL7 does not execute the simulation process properly
.NOTPARALLEL:

COSIM_TOP_DIR   	= $(strip $(shell cd ../..; pwd))
REPO_TOP_DIR 		= $(strip $(shell cd ${COSIM_TOP_DIR}/../..; pwd))
TEST_SUITE_DIR		= $(strip $(shell cd .; pwd))
TEST_SUITE_NAME		= $(shell cd .; basename `pwd`)

default: usage

# Include current test suite common.make
include ./common.make

ISA_SINGLE_CORE_LIST =			\
	rv64mi-p-access				\
	rv64mi-p-breakpoint			\
	rv64mi-p-csr				\
	rv64mi-p-illegal			\
	rv64mi-p-ma_addr			\
	rv64mi-p-ma_fetch			\
	rv64mi-p-mcsr				\
	rv64mi-p-sbreak				\
	rv64mi-p-scall				\
	rv64si-p-csr				\
	rv64si-p-dirty				\
	rv64si-p-icache-alias		\
	rv64si-p-ma_fetch			\
	rv64si-p-sbreak				\
	rv64si-p-scall				\
	rv64si-p-wfi				\
	rv64ua-p-amoadd_d			\
	rv64ua-p-amoadd_w			\
	rv64ua-p-amoand_d			\
	rv64ua-p-amoand_w			\
	rv64ua-p-amomax_d			\
	rv64ua-p-amomaxu_d			\
	rv64ua-p-amomaxu_w			\
	rv64ua-p-amomax_w			\
	rv64ua-p-amomin_d			\
	rv64ua-p-amominu_d			\
	rv64ua-p-amominu_w			\
	rv64ua-p-amomin_w			\
	rv64ua-p-amoor_d			\
	rv64ua-p-amoor_w			\
	rv64ua-p-amoswap_d			\
	rv64ua-p-amoswap_w			\
	rv64ua-p-amoxor_d			\
	rv64ua-p-amoxor_w			\
	rv64ua-p-lrsc				\
	rv64uf-p-fadd				\
	rv64uf-p-fclass				\
	rv64uf-p-fcmp				\
	rv64uf-p-fcvt				\
	rv64uf-p-fcvt_w				\
	rv64uf-p-fdiv				\
	rv64uf-p-fmadd				\
	rv64uf-p-fmin				\
	rv64uf-p-ldst				\
	rv64uf-p-move				\
	rv64uf-p-recoding			\
	rv64ui-p-add				\
	rv64ui-p-addi				\
	rv64ui-p-addiw				\
	rv64ui-p-addw				\
	rv64ui-p-and				\
	rv64ui-p-andi				\
	rv64ui-p-auipc				\
	rv64ui-p-beq				\
	rv64ui-p-bge				\
	rv64ui-p-bgeu				\
	rv64ui-p-blt				\
	rv64ui-p-bltu				\
	rv64ui-p-bne				\
	rv64ui-p-fence_i			\
	rv64ui-p-jal				\
	rv64ui-p-jalr				\
	rv64ui-p-lb					\
	rv64ui-p-lbu				\
	rv64ui-p-ld					\
	rv64ui-p-lh					\
	rv64ui-p-lhu				\
	rv64ui-p-lui				\
	rv64ui-p-lw					\
	rv64ui-p-lwu				\
	rv64ui-p-or					\
	rv64ui-p-ori				\
	rv64ui-p-sb					\
	rv64ui-p-sd					\
	rv64ui-p-sh					\
	rv64ui-p-simple				\
	rv64ui-p-sll				\
	rv64ui-p-slli				\
	rv64ui-p-slliw				\
	rv64ui-p-sllw				\
	rv64ui-p-slt				\
	rv64ui-p-slti				\
	rv64ui-p-sltiu				\
	rv64ui-p-sltu				\
	rv64ui-p-sra				\
	rv64ui-p-srai				\
	rv64ui-p-sraiw				\
	rv64ui-p-sraw				\
	rv64ui-p-srl				\
	rv64ui-p-srli				\
	rv64ui-p-srliw				\
	rv64ui-p-srlw				\
	rv64ui-p-sub				\
	rv64ui-p-subw				\
	rv64ui-p-sw					\
	rv64ui-p-xor				\
	rv64ui-p-xori				\
	rv64um-p-div				\
	rv64um-p-divu				\
	rv64um-p-divuw				\
	rv64um-p-divw				\
	rv64um-p-mul				\
	rv64um-p-mulh				\
	rv64um-p-mulhsu				\
	rv64um-p-mulhu				\
	rv64um-p-mulw				\
	rv64um-p-rem				\
	rv64um-p-remu				\
	rv64um-p-remuw				\
	rv64um-p-remw				\
	rv64uc-p-rvc				\
	rv64ud-p-fadd				\
	rv64ud-p-fclass				\
	rv64ud-p-fcmp				\
	rv64ud-p-fcvt				\
	rv64ud-p-fcvt_w				\
	rv64ud-p-fdiv				\
	rv64ud-p-fmadd				\
	rv64ud-p-fmin				\
	rv64ud-p-ldst				\
	rv64ud-p-move				\
	rv64ud-p-recoding			\
	rv64ud-p-structural

ISA_VIRTUAL_TEST_LIST = 		\
	rv64ui-v-add 				\
	rv64ui-v-addi 				\
	rv64ui-v-addiw 				\
	rv64ui-v-addw 				\
	rv64ui-v-and 				\
	rv64ui-v-andi 				\
	rv64ui-v-auipc 				\
	rv64ui-v-beq 				\
	rv64ui-v-bge 				\
	rv64ui-v-bgeu 				\
	rv64ui-v-blt 				\
	rv64ui-v-bltu 				\
	rv64ui-v-bne 				\
	rv64ui-v-fence_i 			\
	rv64ui-v-jal 				\
	rv64ui-v-jalr 				\
	rv64ui-v-lb 				\
	rv64ui-v-lbu 				\
	rv64ui-v-ld 				\
	rv64ui-v-lh 				\
	rv64ui-v-lhu 				\
	rv64ui-v-lui 				\
	rv64ui-v-lw 				\
	rv64ui-v-lwu 				\
	rv64ui-v-or 				\
	rv64ui-v-ori 				\
	rv64ui-v-sb 				\
	rv64ui-v-sd 				\
	rv64ui-v-sh 				\
	rv64ui-v-simple 			\
	rv64ui-v-sll 				\
	rv64ui-v-slli 				\
	rv64ui-v-slliw 				\
	rv64ui-v-sllw 				\
	rv64ui-v-slt 				\
	rv64ui-v-slti 				\
	rv64ui-v-sltiu 				\
	rv64ui-v-sltu 				\
	rv64ui-v-sra 				\
	rv64ui-v-srai 				\
	rv64ui-v-sraiw 				\
	rv64ui-v-sraw 				\
	rv64ui-v-srl 				\
	rv64ui-v-srli 				\
	rv64ui-v-srliw 				\
	rv64ui-v-srlw 				\
	rv64ui-v-sub 				\
	rv64ui-v-subw 				\
	rv64ui-v-sw 				\
	rv64ui-v-xor 				\
	rv64ui-v-xori 				\
	rv64um-v-div 				\
	rv64um-v-divu 				\
	rv64um-v-divuw 				\
	rv64um-v-divw 				\
	rv64um-v-mul 				\
	rv64um-v-mulh 				\
	rv64um-v-mulhsu 			\
	rv64um-v-mulhu 				\
	rv64um-v-mulw 				\
	rv64um-v-rem 				\
	rv64um-v-remu 				\
	rv64um-v-remuw 				\
	rv64um-v-remw 				\
	rv64ua-v-amoadd_d 			\
	rv64ua-v-amoadd_w 			\
	rv64ua-v-amoand_d 			\
	rv64ua-v-amoand_w 			\
	rv64ua-v-amomax_d 			\
	rv64ua-v-amomaxu_d 			\
	rv64ua-v-amomaxu_w 			\
	rv64ua-v-amomax_w 			\
	rv64ua-v-amomin_d 			\
	rv64ua-v-amominu_d 			\
	rv64ua-v-amominu_w 			\
	rv64ua-v-amomin_w 			\
	rv64ua-v-amoor_d 			\
	rv64ua-v-amoor_w 			\
	rv64ua-v-amoswap_d 			\
	rv64ua-v-amoswap_w 			\
	rv64ua-v-amoxor_d 			\
	rv64ua-v-amoxor_w 			\
	rv64ua-v-lrsc 				\
	rv64uc-v-rvc 				\
	rv64ud-v-fadd 				\
	rv64ud-v-fclass 			\
	rv64ud-v-fcmp 				\
	rv64ud-v-fcvt 				\
	rv64ud-v-fcvt_w 			\
	rv64ud-v-fdiv 				\
	rv64ud-v-fmadd 				\
	rv64ud-v-fmin 				\
	rv64ud-v-ldst 				\
	rv64ud-v-move 				\
	rv64ud-v-recoding 			\
	rv64ud-v-structural 		\
	rv64uf-v-fadd 				\
	rv64uf-v-fclass 			\
	rv64uf-v-fcmp 				\
	rv64uf-v-fcvt 				\
	rv64uf-v-fcvt_w 			\
	rv64uf-v-fdiv 				\
	rv64uf-v-fmadd 				\
	rv64uf-v-fmin 				\
	rv64uf-v-ldst 				\
	rv64uf-v-move 				\
	rv64uf-v-recoding 

TEST_LIST 	= ${ISA_SINGLE_CORE_LIST} ${ISA_VIRTUAL_TEST_LIST}

# Special Debug Module test (temporarily removed)
#OPENOCD_CHECK := $(filter Debugger, $(shell openocd --version 2>&1 | grep Debugger))
#ifeq "$(OPENOCD_CHECK)" "Debugger"
#	TEST_LIST += dtmTest
#endif

runAll: cleanSuite ${TEST_SUITE_DIR}/.ISACreated 
	@for i in ${TEST_LIST}; do 									\
		echo "    PEND : ${TEST_SUITE_DIR}/$${i}"  > ${TEST_SUITE_DIR}/$${i}/status;		\
		make -C ${TEST_SUITE_DIR}/$${i} all; 			\
	done

summary:
	@date > testSuiteSummary
	@for i in ${TEST_LIST}; do				\
		cat ${TEST_SUITE_DIR}/$${i}/status | tee -a testSuiteSummary;	\
	done

makeIsaTest: ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/${TEST_NAME}${SFX}.dump
	@echo "Done"

# Default parameter values, override at commandine ONLY!!!
SINGLE_CORE_ONLY 	?= 0
VIRTUAL_MODE 		?= 0
PASS_WRITETOHOST 	?= 0
MAX_TIMEOUT 		?= 200

${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/${TEST_NAME}${SFX}.dump : ${RISCV_TEST_DIR}/isa/${TEST_NAME}
	@if test ! -d ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}; then	\
		mkdir  ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}; \
	fi
	@cp -f ${ISA_TEST_TEMPLATE}/Makefile        ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/.
	@cp -f ${ISA_TEST_TEMPLATE}/*.h             ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/.
	@rm -f ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc

	@echo "#define MAX_TIMEOUT 100"    						>> ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc

ifneq (${SINGLE_CORE_ONLY},)
	@echo "#define SINGLE_CORE_ONLY" 						>> ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/c_module.cc
endif

	@cat ${ISA_TEST_TEMPLATE}/c_dispatch.cc 				>> ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/c_dispatch.cc
	@cat ${ISA_TEST_TEMPLATE}/c_module.cc 					>> ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/c_module.cc
	@cp -f ${RISCV_TEST_DIR}/isa/${TEST_NAME}      ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/riscv_wrapper.elf
	${RISCV_OBJCOPY} -O binary --change-addresses=-0x80000000 ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/riscv_wrapper.elf ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/riscv_wrapper.img
	@cp -f ${RISCV_TEST_DIR}/isa/${TEST_NAME}.dump ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/${TEST_NAME}${SFX}.dump
	@${BIN_DIR}/createPassFail.pl ${RISCV_TEST_DIR}/isa/${TEST_NAME}.dump ${ISA_SUITE_DIR}/${TEST_NAME}${SFX}/PassFail.hex

# Recursively calls make to build the test directories for ALL of the tests in the lists specified above.
# Given differing test requirements, they are called with different arguments.
.PHONY: createISATests removeISATests
createISATests: removeISATests
	@for i in ${ISA_SINGLE_CORE_LIST}; do 											\
		echo "== Generating Single Core test directory for $${i}";					\
		make TEST_NAME=$${i} SINGLE_CORE_ONLY=1 makeIsaTest; 						\
	done

	@for i in ${ISA_VIRTUAL_TEST_LIST}; do 											\
		echo "== Generating Virtual test directory for $${i}";						\
		make TEST_NAME=$${i} makeIsaTest; 											\
	done

	touch ${TEST_SUITE_DIR}/.ISACreated

# Cleans 
removeISATests:
	rm -rf rv64*
	rm -f ${TEST_SUITE_DIR}/.ISACreated


${TEST_SUITE_DIR}/.ISACreated:
	@echo "CEP_COSIM: Please run 'make createISATests' to continue"
	@exit 1
