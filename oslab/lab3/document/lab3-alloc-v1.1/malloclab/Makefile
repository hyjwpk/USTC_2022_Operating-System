#
# Students' Makefile for the Malloc Lab
#

CC = gcc -g -fPIC 
CFLAGS = -Wall

all: libmem.so

libmem.so: memlib.o mm.o
	$(CC) $(CFLAGS) -shared -o libmem.so mm.o memlib.o

memlib.o: memlib.c memlib.h
mm.o: mm.c mm.h memlib.h

clean:
	rm -f *~ *.o libmem.so


