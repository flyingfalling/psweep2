
#include <unit_tests/cuda_prog.h>

#include <cuda.h>
#include <cuda_runtime.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>

#include <commontypes.h>

#include <helper_cuda.h>


std::vector<size_t> find_legaldevs()
{
  std::vector<size_t> ret;

  
  int deviceCount = 0;
  cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
  if (error_id != cudaSuccess)
    {
      fprintf(stderr, "cudaGetDeviceCount returned %d\n-> %s\n", (int)error_id, cudaGetErrorString(error_id));
      fprintf(stderr, "Result = FAIL\n");
      exit(EXIT_FAILURE);
    }
  
  //  int driverVersion = 0;
  //  int runtimeVersion = 0;

  for (int dev = 0; dev < deviceCount; ++dev)
    {
      cudaSetDevice(dev);
      cudaDeviceProp deviceProp;
      cudaGetDeviceProperties(&deviceProp, dev);
//      printf("\nDevice %d: \"%s\"\n", dev, deviceProp.name);
      /*
	char msg[256];
        SPRINTF(msg, "  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n",
	(float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
        printf("%s", msg);
	printf("  (%2d) Multiprocessors, (%3d) CUDA Cores/MP:     %d CUDA Cores\n",
	deviceProp.multiProcessorCount,
	_ConvertSMVer2Cores(deviceProp.major, deviceProp.minor),
	_ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount);
      */
      if( std::string(deviceProp.name).compare( "Tesla K80" ) == 0 )
	{
	  ret.push_back( dev );
	}
    }
  //cudaDeviceReset();
  return ret;
}

__global__
//void compDist( float64_t *res, float64_t *a, float64_t *b, int sizen )
void compDist( double *res, double *a, double *b, int sizen )
{
  // Get our global thread ID
  int id = (blockIdx.x*blockDim.x) + threadIdx.x;
  
  // Make sure we do not go out of bounds
  if(id < sizen)
    {
      double c = a[id] - b[id];
      res[id] = (c*c);
    }
  //else
  //   { do nothing } 

  //return;
}

std::vector<float64_t> gpucomp( std::vector<float64_t>& est, std::vector<float64_t>& actual, size_t& cudadevnum )
{
  checkCudaErrors(cudaSetDevice(cudadevnum)); //check errors? rofl.
  
  if(est.size() != actual.size())
    {
      fprintf(stderr,"REV: ERROR in cuda gpucomp, actual != est size!\n"); exit(1);
    }

  std::vector<float64_t> result( est.size(), -666 );
  
  float64_t* d_estptr;
  float64_t* d_actualptr;
  float64_t* d_resultptr;
  
  //Run the appropriate kernel
  checkCudaErrors(cudaMalloc(&d_estptr, est.size()*sizeof(est[0]) ));
  checkCudaErrors(cudaMalloc(&d_actualptr, actual.size()*sizeof(actual[0]) ));
  checkCudaErrors(cudaMalloc(&d_resultptr, result.size()*sizeof(result[0]) ));
  
  checkCudaErrors(cudaMemcpy(d_estptr, est.data(), est.size()*sizeof(est[0]), 
	     cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMemcpy(d_actualptr, actual.data(), actual.size()*sizeof(actual[0]), 
	     cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMemcpy(d_resultptr, result.data(), result.size()*sizeof(result[0]), 
	     cudaMemcpyHostToDevice));

  //RUN KERNEL
  int blockSize=0;
  int gridSize=0;
 
  // Number of threads in each thread block
  blockSize = 128;
 
  // Number of thread blocks in grid
  gridSize = 1; //(int)ceil((float)est.size()/blockSize);
  
  // Execute the kernel
  compDist<<<gridSize, blockSize>>>(d_resultptr, d_estptr, d_actualptr, (int)result.size());

  //REV: Do I need to synch it or something?
  cudaDeviceSynchronize() ;

  checkCudaErrors(cudaMemcpy( result.data(), d_resultptr, result.size()*sizeof(result[0]), cudaMemcpyDeviceToHost ));

  checkCudaErrors(cudaFree( d_estptr ));
		  checkCudaErrors(cudaFree( d_actualptr ));
  checkCudaErrors( cudaFree( d_resultptr ) );

  getLastCudaError("REV: Kernel execution failed");
  
  return result;  
  
}