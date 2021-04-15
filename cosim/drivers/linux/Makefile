#************************************************************************
# Copyright 2021 Massachusetts Institute of Technology
#
# File Name:      Makefile
# Program:        Common Evaluation Platform (CEP)
# Description:    Cross-compiler Makefile for CEP dispatcher
#************************************************************************

TARGET = cep_diag
FREEDOM_U_SDK_DIR = ../../../software/freedom-u-sdk
BUILDROOT_DIR = $(FREEDOM_U_SDK_DIR)/buildroot
PACKAGE_DIR = $(BUILDROOT_DIR)/package/$(TARGET)

LIBS = -lssl -lcrypto
#
# top level Makefile override this CC to use gcc BUT we need g++!!!
#
CC = riscv64-unknown-linux-gnu-gcc
# skip on error
SOE = 0
#MY_CC = /home/aduong/CEP/CEP-master/software/freedom-u-sdk/work/buildroot_initramfs/host/usr/bin/riscv64-unknown-linux-gnu-g++
#
# So force it to use g++ instead
#
MY_CC = $(patsubst %gcc, %g++, $(CC))
CFLAGS += -g -Wall -I . -DLINUX_MODE -DCEP_SOE=$(SOE) -DRISCV_CPU -DBIG_ENDIAN
CFLAGS += -std=c++11 
# -static will crash for mutithread
LFLAGS = -lpthread

#
# for local build testing ONLY
#
LOCAL=0
ifeq (${LOCAL},1)
SOE = 1
MY_CC = g++
CFLAGS += -DLOCAL_MACHINE
endif

#
# 
#
FILE_LIST = $(wildcard *.cc *.h Makefile)
#
# Other files
#
OTHER_FILES = $(wildcard ../diag/*.cc ../diag/*.h) 				\
	../../include/cep_adrMap.h ../../include/v2c_cmds.h 			\
	$(wildcard ../../simDiag/*.cc ../../simDiag/*.h) 			\
	../cep_tests/*.cc ../cep_tests/*.h        				\
	../../share/portable_io.h ../../share/access.cc ../../share/access.h	\
	../bare/encoding.h \


.PHONY: clean install append

OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(MY_CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(MY_CC) $(OBJECTS) -Wall $(LIBS) -o $@ $(LFLAGS)

#
# These make targets are used to add the application to the freedom-u-sdk
# build system.  
#
# install - Copy all the relevant files to the freedom-u-sdk/builtroot/package
#   directory.  The can be re-run every time the original source is updated.
#
# append  - Used to modify the buildroot package Config.in to properly pull in
#   this application.  In addition, it will enabled the package in the buildroot_initramfs_config
#   (since it seems that freedom-u-sdk does not mount the rootfs).  Should only be run once.
#   If multiple applications are added using this method, the $(BUILDROOT_DIR)/package/Config.in
#   will need to be inspected and "cleaned up" a bit to be proper
#
install: 

# Remove, then create the target package directory
	@echo
	@echo "--- Installing $(TARGET) to $(BUILDROOT_DIR) ---"
	@echo
	rm -rf $(PACKAGE_DIR)
	mkdir $(PACKAGE_DIR)
	mkdir $(PACKAGE_DIR)/src

# Copy the Config.in and makefile to the package directory
	cp Config.in $(PACKAGE_DIR)
	cp $(TARGET).mk $(PACKAGE_DIR)

# Copy the source to the source directory
	cp $(FILE_LIST) $(PACKAGE_DIR)/src
	cp $(OTHER_FILES) $(PACKAGE_DIR)/src
	@echo
	@echo "--- Installation Complete ---"
	@echo

#
# Only if not yet done so
#
CONFIG_GREP = $(shell grep 'package/$(TARGET)/Config.in' $(BUILDROOT_DIR)/package/Config.in)

append:
ifeq ($(CONFIG_GREP),)
# First, delete the last "endmenu" from Config.in
	@echo
	@echo "--- Adding $(TARGET) to $(BUILDROOT_DIR)/package/Config.in ---"
	@echo
	cat $(BUILDROOT_DIR)/package/Config.in | head -n -1 > tmp.txt
	cp tmp.txt $(BUILDROOT_DIR)/package/Config.in
	rm -f tmp.txt

# Append the CEP Applications menu item to Config.in
	echo "" >> $(BUILDROOT_DIR)/package/Config.in
	echo 'menu "CEP Tests"' >> $(BUILDROOT_DIR)/package/Config.in
	echo '        source "package/$(TARGET)/Config.in"' >> $(BUILDROOT_DIR)/package/Config.in
	echo "endmenu" >> $(BUILDROOT_DIR)/package/Config.in
	echo "" >> $(BUILDROOT_DIR)/package/Config.in
	echo "endmenu" >> $(BUILDROOT_DIR)/package/Config.in	# Replacing the endmenu we deleted above
# Enable the CEP Regression suite in the Freedom U SDK Root File System
	@echo
	@echo "--- Enabling application in $(FREEDOM_U_SDK_DIR)/conf/buildroot_initramfs_config ---"
	@echo
	echo "BR2_PACKAGE_OPENSSL=y" >> $(FREEDOM_U_SDK_DIR)/conf/buildroot_initramfs_config
	echo "BR2_PACKAGE_CEP_DIAG=y" >> $(FREEDOM_U_SDK_DIR)/conf/buildroot_initramfs_config
	@echo
	@echo "--- Append Complete ---"
	@echo
else
	@echo "--- No need to append since already appended ---"
endif

clean:
	rm -f *.o
	rm -f $(TARGET)

