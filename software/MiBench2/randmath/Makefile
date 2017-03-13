default : main.elf

randmath : randmath.c
	gcc -o randmath randmath.c

abcmath.c : randmath
	./randmath

OBJS = abcmath.o notmain.o

include ../Makefile.mk

more_clean:
	rm -rf randmath abcmath.c

