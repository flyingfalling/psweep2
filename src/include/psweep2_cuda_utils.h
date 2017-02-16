
#pragma once

#include <commontypes.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>

#ifdef CUDA_SUPPORT
#include <psweep2_cuda_functs_impl.h>
#endif

//Problem is this will depend on workersperguy
size_t compute_gpu_idx( const size_t& localidx, const size_t& nworkersperrank, const size_t& ranknum );

void set_cuda_device(const size_t& idx);
