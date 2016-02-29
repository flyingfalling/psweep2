CC=g++
MPICC=mpic++
HDF5HEADERS=/usr/include/hdf5/serial
INCLUDEFLAG=-I./ -I./include -I$(HDF5HEADERS)
CFLAGS=-std=c++11 -O3 -g #-Wall
CPPDIR=./cpp
INCLUDEDIR=./include
UNITTESTDIR=./unit_tests
LIBS=-L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial

#default: scriptparser2.exe
#default: test_cmd_script_parser.exe
#default: test_parampoint.exe
#default: test_searchalgo.exe
#default: test_dream_abc_z.exe test_dream_abc.exe
default: test_psweep2_lib.exe

HEADERFILES=$(wildcard $(INCLUDEDIR)/*)
CPPFILES=$(wildcard $(CPPDIR)/*)

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

test_searchalgo.exe: $(UNITTESTDIR)/test_searchalgo.cpp $(HEADERFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_searchalgo.cpp -o test_searchalgo.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system

test_hdf5collection.exe: $(UNITTESTDIR)/test_hdf5collection.cpp $(HEADERFILES)
	h5c++ --verbose $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_hdf5collection.cpp -o test_hdf5collection.exe

test_memfile.exe: $(UNITTESTDIR)/test_memfile.cpp $(HEADERFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_memfile.cpp -o test_memfile.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system

#test_boostio.exe: $(UNITTESTDIR)/test_memfile.cpp $(HEADERFILES)
#	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_boostio.cpp -o test_boostio.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system

test_memfile2.exe: $(UNITTESTDIR)/test_memfile2.cpp $(HEADERFILES)
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_memfile2.cpp -o test_memfile2.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system


test_memfile3.exe: $(UNITTESTDIR)/test_memfile3.cpp $(HEADERFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(UNITTESTDIR)/test_memfile3.cpp -o test_memfile3.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system


test_fakesys.exe:  $(UNITTESTDIR)/test_fakesys.cpp $(HEADERFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(LIBS) $(UNITTESTDIR)/test_fakesys.cpp -o test_fakesys.exe -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system  -lhdf5_cpp -lhdf5 -lhdf5_serial


#hdf5_collection.o: $(HEADERFILES) $(INCLUDEDIR)/hdf5_collection.cpp
#	export HDF5_CXX=$(MPICC)
#	h5c++ -c --verbose $(CFLAGS) $(INCLUDEFLAG) $(INCLUDEDIR)/hdf5_collection.cpp

test_dream_abc.exe: $(UNITTESTDIR)/test_dream_abc.cpp $(HEADERFILES) $(CPPFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(LIBS) -o test_dream_abc.exe $(UNITTESTDIR)/test_dream_abc.cpp -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lhdf5_cpp -lhdf5


test_dream_abc_z.exe: $(UNITTESTDIR)/test_dream_abc_z.cpp $(HEADERFILES) $(CPPFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(LIBS) -o test_dream_abc_z.exe $(UNITTESTDIR)/test_dream_abc_z.cpp -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lhdf5_cpp -lhdf5

test_dreamabc_enum.exe: $(UNITTESTDIR)/test_dreamabc_enum.cpp $(HEADERFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(LIBS) -o test_dreamabc_enum.exe $(UNITTESTDIR)/test_dreamabc_enum.cpp -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lhdf5_cpp -lhdf5

runtestabc: test_dream_abc.exe
	mpirun -n 2 ./test_dream_abc.exe 1>stdout0 2>stderr0 &

runtestabcz: test_dream_abc_z.exe
	mpirun -n 2 ./test_dream_abc_z.exe 1>stdout0 2>stderr0 &


testhdf5id.exe: $(UNITTESTDIR)/testhdf5id.cpp
	h5c++ $(CFLAGS) $(UNITTESTDIR)/testhdf5id.cpp -o testhdf5id.exe

test_optionparse.exe: $(UNITTESTDIR)/test_optionparse.cpp $(HEADERFILES)
	g++ -std=c++11 $(INCLUDEFLAG) $(UNITTESTDIR)/test_optionparse.cpp -o test_optionparse.exe

test_optionhandler.exe: $(UNITTESTDIR)/test_optionhandler.cpp $(HEADERFILES)
	g++ -std=c++11 $(INCLUDEFLAG) $(UNITTESTDIR)/test_optionhandler.cpp -o test_optionhandler.exe

test_psweep2_lib.exe: $(UNITTESTDIR)/test_psweep2_lib.cpp $(HEADERFILES) $(CPPFILES)
	$(MPICC) $(CFLAGS) $(INCLUDEFLAG) $(LIBS) -o test_psweep2_lib.exe $(UNITTESTDIR)/test_psweep2_lib.cpp -lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lhdf5_cpp -lhdf5



clean:
	rm *.exe *.o
