all : host jpegdata.h main.elf


host : host.c picojpeg.c picojpeg.h jpegdata.h
	gcc -O2 host.c picojpeg.c -o host

jpegdata.h : bintoc.c
	gcc -O2 bintoc.c -o bintoc
	./bintoc

more_clean :
	rm -rf host out.bmp bintoc jpegdata.h

OBJS = uart.o picojpeg.o picojpegtest.o embedded.o

include ../Makefile.mk