CC=g++
MPICC=mpic++
INCLUDEFLAG=-I./ -I./include
CFLAGS=-std=c++11 -O3
INCLUDEDIR=./include
UNITTESTDIR=./unit_tests

#default: scriptparser2.exe
#default: test_cmd_script_parser.exe
default: test_parampoint.exe

HEADERFILES=$(INCLUDEDIR)/$*

testvariable.exe: $(INCLUDEDIR)/variable.h $(UNITTESTDIR)/testvariable.cpp 
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/testvariable.cpp -o testvariable.exe

testhvar.exe: $(INCLUDEDIR)/variable.h $(INCLUDEDIR)/hierarchical_varlist.h $(UNITTESTDIR)/testhvar.cpp
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/testhvar.cpp -o testhvar.exe

scriptparser2.exe: $(UNITTESTDIR)/scriptparser2.cpp $(INCLUDEDIR)/script_unroller.h $(INCLUDEDIR)/utility_functs.h
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/scriptparser2.cpp -o scriptparser2.exe

test_cmd_script_parser.exe: $(UNITTESTDIR)/test_cmd_script_parser.cpp $(INCLUDEDIR)/psweep_cmd_script_parser.h $(INCLUDEDIR)/script_unroller.h
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_cmd_script_parser.cpp -o test_cmd_script_parser.exe

testfunctrep2.exe: $(INCLUDEDIR)/psweep_cmd_script_parser.h $(INCLUDEDIR)/script_unroller.h $(INCLUDEDIR)/variable.h $(INCLUDEDIR)/hierarchical_varlist.h $(INCLUDEDIR)/functrep.h   $(UNITTESTDIR)/testfunctrep2.cpp $(INCLUDEDIR)/utility_functs.h
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/testfunctrep2.cpp -o testfunctrep2.exe

testfunctrep.exe: $(INCLUDEDIR)/psweep_cmd_script_parser.h $(INCLUDEDIR)/script_unroller.h $(INCLUDEDIR)/variable.h $(INCLUDEDIR)/hierarchical_varlist.h $(INCLUDEDIR)/functrep.h   $(UNITTESTDIR)/testfunctrep.cpp
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/testfunctrep.cpp -o testfunctrep.exe

test_boostmpi.exe: $(UNITTESTDIR)/test_boostmpi.cpp
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_boostmpi.cpp -o test_boostmpi.exe -lboost_mpi

test_parampoint.exe: $(UNITTESTDIR)/test_parampoint.cpp $(HEADERFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_parampoint.cpp -o test_parampoint.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system

clean:
	rm *.exe *.o
