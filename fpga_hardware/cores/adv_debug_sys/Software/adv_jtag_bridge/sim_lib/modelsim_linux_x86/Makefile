
SRCDIR=../src/
CC = gcc

# The location of the ModelSim installation, used
# to build the C lib for VPI.  Must contain a subdir with
# vpi_user.h.
MODEL=/opt/modelsim



default: vpi

# This is for ModelSim under Linux (x86/32)
# If RH v7.1 or lower, add -noinhibit-exec
jp-io-vpi.so: $(SRCDIR)jp-io-vpi.c Makefile
	$(CC) -c -I$(MODEL)/modeltech/include $(SRCDIR)jp-io-vpi.c -o jp-io-vpi.o
	$(CC) -shared -Bsymbolic  -o jp-io-vpi.so jp-io-vpi.o -lc

vpi: jp-io-vpi.so Makefile