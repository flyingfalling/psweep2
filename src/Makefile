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
CUDALIBS=-lcuda -lcudart
SHAREDLIBFLAG=-fPIC
CUDALD=-L/usr/lib/nvidia-cuda-toolkit
NVCC=nvcc
#GENCODE=-gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52
GENCODE=-gencode arch=compute_37,code=sm_37
#REV: had to add the -D_FORCE_INLINES for some nasty stuff with gcc version being too far ahead...
NVCCFLAGS=-m64 -g -G -DDEBUG=6 $(GENCODE) -D_FORCE_INLINES -ccbin=$(CXX) -Xcompiler -fPIC
NVCCSHAREDLIBFLAG=-Xcompiler '-fPIC'

CUDADIR=./cudaimpl

INSTALL_PATH=/usr/local

headers = $(wildcard (INCLDIR)/*.h)
srcs = $(wildcard $(CPPDIR)/*.cpp)
objs = $(srcs:.cpp=.o)
deps = $(srcs:.cpp=.d)


DEF_CUDA_SUPPORT= #"YES"


ifdef DEF_CUDA_SUPPORT
CPPFLAG += -DCUDA_SUPPORT
NVCCFLAGS += -DCUDA_SUPPORT
#srcs += $(wildcard $(CUDADIR)/*.cu)
endif

default: libpsweep2.so

install: libpsweep2.so
	mkdir -p "$(INSTALL_PATH)/lib"
	cp libpsweep2.so $(INSTALL_PATH)/lib
	ldconfig -v

libpsweep2.so: $(objs) $(headers) Makefile
ifdef DEF_CUDA_SUPPORT
	$(NVCC) $(NVCCSHAREDLIBFLAG) $(CUDALD) $(NVCCFLAGS) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) -c cudaimpl/psweep2_cuda_functs_impl.cu -lcuda -lcudart
	$(MPICC) -shared $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) $(objs) psweep2_cuda_functs_impl.o -o libpsweep2.so $(LIBS)
else
	$(MPICC) -shared $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) $(objs) -o libpsweep2.so $(LIBS)
endif

#test: $(objs)
#	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/test_psweep2_lib.cpp  $(objs) -o test_psweep2_lib.exe $(LIBS)

test: unit_tests/test_psweep2_lib.cpp libpsweep2.so
	$(MPICC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/test_psweep2_lib.cpp -o test_psweep2_lib.exe -lpsweep2 $(LIBS)

gputest: unit_tests/gputest2.cpp libpsweep2.so unit_tests/cuda_prog.cu unit_tests/cuda_prog.h
	$(NVCC) $(CUDALD) $(NVCCFLAGS) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) -c unit_tests/cuda_prog.cu -lcuda -lcudart
	$(MPICC) $(CUDALD) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/gputest2.cpp cuda_prog.o -o gputest.exe -lpsweep2 $(LIBS) $(CUDALIBS)

%.o: %.cpp
	$(MPICC) $(SHAREDLIBFLAG) -MMD -MP -c $(CPPFLAG) $(INCLFLAG) $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) $(deps) test

-include $(deps)
