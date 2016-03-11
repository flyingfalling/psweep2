CC = g++
MPICC = mpic++
H5INCL=/usr/include/hdf5/serial
INCLDIR=./include
IMPLDIR=./impl
CPPDIR=./cpp
CPPFLAG=-std=c++11 -O3 #-g -Wall
INCLFLAG=-I. -I$(INCLDIR) -I$(IMPLDIR) -I$(H5INCL)
LIBFLAG=-L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial
LIBS=-lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lhdf5_cpp -lhdf5
SHAREDLIBFLAG=-fPIC


srcs = $(wildcard $(CPPDIR)/*.cpp)
objs = $(srcs:.cpp=.o)
deps = $(srcs:.cpp=.d)

default: test

lib.so: $(objs)
	$(MPICC) -shared $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) $(objs) -o lib.so $(LIBS)

#test: $(objs)
#	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/test_psweep2_lib.cpp  $(objs) -o test_psweep2_lib.exe $(LIBS)

test: unit_tests/test_psweep2_lib.cpp lib.so
	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/test_psweep2_lib.cpp -o test_psweep2_lib.exe lib.so $(LIBS)

%.o: %.cpp
	$(MPICC) $(SHAREDLIBFLAG) -MMD -MP -c $(CPPFLAG) $(INCLFLAG) $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) $(deps) test

-include $(deps)
