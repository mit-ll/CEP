#************************************************************************
# Copyright (C) 2019 Massachusetts Institute of Technology
#
# File Name:      Makefile
# Program:        Common Evaluation Platform (CEP)
# Description:    Cross-compiler Makefile for CEP regression suite
#************************************************************************

TARGET = cepregression
FREEDOM_U_SDK_DIR = ../../freedom-u-sdk
BUILDROOT_DIR = $(FREEDOM_U_SDK_DIR)/buildroot
PACKAGE_DIR = $(BUILDROOT_DIR)/package/$(TARGET)

LIBS =
CC = riscv64-unknown-linux-gnu-g++
CFLAGS = -g -Wall -I . 
LFLAGS = -static

FILE_LIST = $(wildcard *.c *.h *.cpp Makefile)

.PHONY: clean install append

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@ $(LFLAGS)

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
	@echo
	@echo "--- Installation Complete ---"
	@echo

append:
# First, delete the last "endmenu" from Config.in
	@echo
	@echo "--- Adding $(TARGET) to $(BUILDROOT_DIR)/package/Config.in ---"
	@echo
	cat $(BUILDROOT_DIR)/package/Config.in | head -n -1 > tmp.txt
	cp tmp.txt $(BUILDROOT_DIR)/package/Config.in
	rm -f tmp.txt

# Append the CEP Applications menu item to Config.in
	echo "" >> $(BUILDROOT_DIR)/package/Config.in
	echo 'menu "CEP Applications"' >> $(BUILDROOT_DIR)/package/Config.in
	echo '        source "package/$(TARGET)/Config.in"' >> $(BUILDROOT_DIR)/package/Config.in
	echo "endmenu" >> $(BUILDROOT_DIR)/package/Config.in
	echo "" >> $(BUILDROOT_DIR)/package/Config.in
	echo "endmenu" >> $(BUILDROOT_DIR)/package/Config.in	# Replacing the endmenu we deleted above
	@echo
	@echo "--- Enabling application in $(FREEDOM_U_SDK_DIR)/conf/buildroot_initramfs_config ---"
	@echo

# Enable the CEP Regression suite in the Freedom U SDK Root File System
	@echo
	@echo "--- Enabling application in $(FREEDOM_U_SDK_DIR)/conf/buildroot_initramfs_config ---"
	@echo
	echo "BR2_PACKAGE_CEPREGRESSION=y" >> $(FREEDOM_U_SDK_DIR)/conf/buildroot_initramfs_config
	@echo
	@echo "--- Append Complete ---"
	@echo

clean:
	rm -f *.o
	rm -f $(TARGET)
