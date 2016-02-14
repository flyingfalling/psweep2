
CC=g++
MPICC=mpic++
INCLUDEFLAG=-I../ -I../include
CFLAGS=-std=c++11 -O3
INCLUDEDIR=./include
UNITTESTDIR=./unit_tests


runtest.exe: runtest.cpp
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) runtest.cpp -o runtest.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system

default: runtest.exe

clean:
	rm *.o *.exe *~
