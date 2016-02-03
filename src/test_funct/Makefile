
CC=g++
MPICC=mpic++
INCLUDEFLAG=-I../ -I../include
CFLAGS=-std=c++11 -O3
INCLUDEDIR=./include
UNITTESTDIR=./unit_tests


runtest.exe: runtest.cpp
	$(CC) $(CFLAGS) $(INCLUDEFLAG) runtest.cpp -o runtest.exe

default: runtest.exe

clean:
	rm *.o *.exe *~
