CC = g++
MPICC = mpic++
H5INCL=/usr/include/hdf5/serial
INCLDIR=./include
IMPLDIR=./impl
CPPDIR=./cpp
CPPFLAG=-std=c++11
INCLFLAG=-I. -I$(INCLDIR) -I$(IMPLDIR) -I$(H5INCL)
LIBFLAG=-L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial
LIBS=-lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lhdf5_cpp -lhdf5

srcs = $(wildcard $(CPPDIR)/*.cpp)
objs = $(srcs:.cpp=.o)
deps = $(srcs:.cpp=.d)

test: $(objs)
	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/test_psweep2_lib.cpp  $(objs) -o test_psweep2_lib.exe $(LIBS)

%.o: %.cpp
	$(MPICC) -MMD -MP -c $(CPPFLAG) $(INCLFLAG) $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) $(deps) test

-include $(deps)
