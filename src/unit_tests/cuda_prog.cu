
#include <unit_tests/cuda_prog.h>

#include <cuda.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>


std::vector<size_t> find_legaldevs()
{
  std::vector<size_t> ret;

  
  int deviceCount = 0;
  cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
  if (error_id != cudaSuccess)
    {
      printf("cudaGetDeviceCount returned %d\n-> %s\n", (int)error_id, cudaGetErrorString(error_id));
      printf("Result = FAIL\n");
      exit(EXIT_FAILURE);
    }
  
  //  int driverVersion = 0;
  //  int runtimeVersion = 0;

  for (int dev = 0; dev < deviceCount; ++dev)
    {
      cudaSetDevice(dev);
      cudaDeviceProp deviceProp;
      cudaGetDeviceProperties(&deviceProp, dev);
 //     printf("\nDevice %d: \"%s\"\n", dev, deviceProp.name);
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
