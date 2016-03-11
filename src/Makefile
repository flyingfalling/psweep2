CC = g++
MPICC = mpic++
H5INCL=/usr/include/hdf5/serial
INCLDIR=./include
IMPLDIR=./impl
CPPDIR=./cpp
CPPFLAG=-std=c++11 -O3 #-g -Wall
INCLFLAG=-I. -I$(INCLDIR) -I$(IMPLDIR) -I$(H5INCL)
LIBFLAG=-L. -L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial
LIBS=-lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lhdf5_cpp -lhdf5
SHAREDLIBFLAG=-fPIC

INSTALL_PATH=/usr/local

srcs = $(wildcard $(CPPDIR)/*.cpp)
objs = $(srcs:.cpp=.o)
deps = $(srcs:.cpp=.d)


default: libpsweep2.so

install: libpsweep2.so
	mkdir -p $(INSTALL_PATH)/lib
	cp libpsweep2.so $(INSTALL_PATH)/lib
	ldconfig -v

libpsweep2.so: $(objs)
	$(MPICC) -shared $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) $(objs) -o libpsweep2.so $(LIBS)

#test: $(objs)
#	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/test_psweep2_lib.cpp  $(objs) -o test_psweep2_lib.exe $(LIBS)

test: unit_tests/test_psweep2_lib.cpp libpsweep2.so
	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/test_psweep2_lib.cpp -o test_psweep2_lib.exe -lpsweep2 $(LIBS)

gputest: unit_tests/gputest.cpp libpsweep2.so
	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/gputest.cpp -o gputest.exe -lpsweep2 $(LIBS)

%.o: %.cpp
	$(MPICC) $(SHAREDLIBFLAG) -MMD -MP -c $(CPPFLAG) $(INCLFLAG) $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) $(deps) test

-include $(deps)
