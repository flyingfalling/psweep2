#include <psweep2_cuda_utils.h>

size_t compute_gpu_idx( const size_t& localidx, const size_t& nworkersperrank, const size_t& ranknum )
{
  size_t mydevidx = localidx; // / nworkersperrank; //local idx is RANK in here. E.g. I only spawned 8...?

  std::string devname = "Tesla K80";

#ifdef CUDA_SUPPORT
  fprintf(stdout, "CUDA SUPPORT: COMPUTING GPU IDX BASED ON ACCEPTABLE DEVICE TYPE [%s]\n", devname.c_str());
  std::vector<size_t> devs = findlegaldevs_byname(devname); //Base it on some requirements of user.
  if( devs.size() <= mydevidx )
    {
      fprintf(stderr, "compute_gpu_idx: ERROR, not enough GPU devices on host (I am [%ld] but there are only [%ld])! (Rank [%ld])\n", mydevidx, devs.size(), ranknum);
      exit(1);
    }
  fprintf(stdout, "Computing local index. My localidx is [%ld], with [%ld] workers per rank. Found [%ld] devs on the machine\n", localidx, nworkersperrank, devs.size());
  mydevidx = devs[mydevidx];
#endif

  return mydevidx;
}


void set_cuda_device(const size_t& idx)
{

#ifdef CUDA_SUPPORT
  real_set_cuda_device(idx);
#endif

}
