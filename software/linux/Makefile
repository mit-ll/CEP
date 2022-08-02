#************************************************************************
# Copyright 2022 Massachusets Institute of Technology
# SPDX short identifier: BSD-2-Clause
#
# File Name:      Makefile
# Program:        Common Evaluation Platform (CEP)
# Description:    
# Notes:          
#
#************************************************************************

REPOROOT     	:= $(shell git rev-parse --show-toplevel)
BASEDIR 		:= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PACKAGE     	= BR2_PACKAGE_$(shell echo $(MAINPROGRAM) | tr '[:lower:]' '[:upper:]')
INSTALL_DIR		= ${REPOROOT}/software/firemarshal/boards/prototype/distros/br/buildroot/package/$(MAINPROGRAM)
BR_CONFIG_IN	= ${REPOROOT}/software/firemarshal/boards/prototype/distros/br/buildroot/package/Config.in
BR_CONFIG		= ${REPOROOT}/software/firemarshal/boards/prototype/base-workloads/br-base/buildroot-config

# Rerform some error checking
ifeq (,${MAINPROGRAM})
$(error Usage make MAINPROGRAM=<prog name> <install | clean>)
endif

ifeq (, $(wildcard ${MAINPROGRAM}))
$(error Directory ${MAINPROGRAM} not found)
endif

info:
	@echo "Usage: make install - Installs application in firemarshal's buildroot"

install:
	@echo "Installing $(MAINPROGRAM) into firemarshal's buildroot..."
ifeq (,$(shell grep ${PACKAGE} ${BR_CONFIG}))
	echo "$(PACKAGE)=y" >> $(BR_CONFIG)
endif	
ifeq (,$(shell grep "package/${MAINPROGRAM}/Config.in" ${BR_CONFIG_IN}))
	sed -i '/xutil_util-macros/i\ \ \ \ source "package\/${MAINPROGRAM}\/Config.in"' $(BR_CONFIG_IN)
endif
	rm -rf $(INSTALL_DIR)
	mkdir $(INSTALL_DIR)
	-cp $(MAINPROGRAM)/Config.in $(MAINPROGRAM)/$(MAINPROGRAM).mk $(INSTALL_DIR)

.PHONY: clean
clean:
	rm -rf $(INSTALL_DIR)
ifneq (,$(shell grep ${PACKAGE} ${BR_CONFIG}))
	sed -i '/$(PACKAGE)/d' $(BR_CONFIG)
endif	
ifneq (,$(shell grep "package/${MAINPROGRAM}/Config.in" ${BR_CONFIG_IN}))
	sed -i '/package\/${MAINPROGRAM}\/Config.in/d' $(BR_CONFIG_IN)
endif
