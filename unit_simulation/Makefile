#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#//
#// File Name:      Makefile     
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Testbench makefile for Unit Level Testbench
#// Notes:          
#// Usage: 
#//
#//    make   		<-- run all cores simulation (cycle-based as well as TL-level-based sims)
#//    make vectors 	<-- prepare stimulus for simulation (only if chip-level sim change)
#//    make clean	<-- clean all output files from simulations
#//
#//************************************************************************
#

#
# Check that the core is one of the supported cores below
#
CORE_STR            = aes:md5:des3:sha256:gps:dft:idft:fir:iir:rsa
#
#
CORE_LIST           = $(subst  :, ,${CORE_STR})
#
#
all:
	@for i in ${CORE_LIST}; do	\
	   (cd $${i}_sim; make ${MAKEFLAGS})	\
	done
	(cd TL_level_sim; make ${MAKEFLAGS} runAll)
	grep TEST *_sim/*.log 

# ------------------------------------------
#
# Execute this manually if there is any change in the captured *.h file
#
# ------------------------------------------
#
C_PLAYBACK_DIR   = ../cosim/drivers/vectors
C_PLAYBACK_FLIST = $(wildcard ${C_PLAYBACK_DIR}/*.txt.gz)
C_PLAYBACK_FILES = $(foreach t,${notdir ${C_PLAYBACK_FLIST}}, ${t})
#
# Copy *_stimulus to appropriate dirs 
#
vectors: 
	@for i in ${CORE_LIST}; do			\
		rm -f $${i}_sim/*_stimulus.txt; 	\
		cp ${C_PLAYBACK_DIR}/$${i}_stimulus.txt.gz $${i}_sim/.; \
	done
	(cd TL_level_sim; make vectors)

#
# Clean up
#
clean:
	@for i in ${CORE_LIST}; do	\
	   (cd $${i}_sim; make clean)	\
	done
	(cd TL_level_sim; make clean)

