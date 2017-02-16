#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <psweep2_cuda_functs_impl.h>

void real_set_cuda_device( const size_t& idx )
{
  checkCudaErrors( cudaSetDevice(idx) );
}

std::vector<size_t> findlegaldevs_byname(const std::string& devname)
{
  std::vector<size_t> ret;
  
  int deviceCount = 0;
 cudaError_t error_id = cudaGetDeviceCount(&deviceCount) ;
  if (error_id != cudaSuccess)
    {
      fprintf(stderr, "cudaGetDeviceCount returned %d\n-> %s\n", (int)error_id, cudaGetErrorString(error_id));
      fprintf(stderr, "Result = FAIL\n");
      exit(EXIT_FAILURE);
    }
  
  for(int dev = 0; dev < deviceCount; ++dev)
    {
      checkCudaErrors( cudaSetDevice(dev) );
      cudaDeviceProp deviceProp;
      checkCudaErrors( cudaGetDeviceProperties(&deviceProp, dev) );
      if( devname.compare( std::string(deviceProp.name) ) == 0 )
	{
	  ret.push_back( dev );
	}
    }

  return ret;
}