//REV: Helper functions to let user do simple CUDA-related tasks such as
//Specify certain requirements for his user program, and know which ordinal
//CUDA device he should use in his program.

//Another way to do it is to spawn single workers that can accept up to
// N jobs asynchronously, and spin them off in cuda, and get results etc.
// I.e. a kind of local farmer on this node. This way we can execute multiple
// kernels in parallel on "large" GPUs if the kernel is small enough. Each
// process will effectively have multiple threads, one for each worker. Kind
// of like a mini MPI. I could use OMP for it, or just normal threads I think,
//but that would require each process getting some # of CPUs.

//This must be transparent to the user. Basically workers start up on each
//node, e.g. a single worker "process". That is openmp, and it has N guys who
//are basically doing mini MPI type thing, with a guy communicating with
//the threads and the other ones each have a specified GPU they work on.
//And there might be many on the same GPU if we are launching multiple kernels.

//This is a much nicer way to handle things, although it makes it more
//difficult as I need within-openMP conversation as well. Also, each oMP guy
//only has some number of CPUs it works on. So I should switch the 8 GPUs to
//the other guy.

//What do I want to do? Let us assume, my model has 10k neurons with
//15 parameters each
//It furthermore has 1000 synapses per 10k neurons, in other words, 10M syns.
//Each synapse is STP/etc., and thus contains 20 parameters each. Assuming
//doubles, we are at 8*10k*20 + 20*10M*8. So, 160*10M. + 160*10k = 1600M, 1.6G
//Plus the 1.6M from the other guys, nothing heh.
//So, that is um, 1.6 GB. Plus all the other stuff...assuming that, we are at
//what, max of 2GB each. We have 24GB I think? So I can actually do massively parallel if I swing this right, depending on the size of the model. There must be some max number of parallel kernels I can run though. And I need the master guy to handle them well.

//Holy shit, this will be so much faster...and it will use the 2400 processors so much more efficiently. Problem will be of course memory bandwidth...
//Let's say I conservatively run 4 on each in parallel. That's already 12*4, i.e. 48 GPUs I can run simultaneously. Even better I can run multiple trials with the same circuit (parameters)...?

//First, do something ghetto to bubble up device number. It will have to be
//the user who spawns another (multiple?) kernel on the device. Shit, I think it has to be from a single thread too (?), so I actually have a bit more complexity because EVERYTHING has to be sent to the GPU from a "GPU farmer thread", although preparation can be done in other openMP threads I assume ;)
//Furthermore, they can prepare other work while waiting for the GPU. I guess I like that, kind of like a mini-scheduler with dependencies. They go up
//to their point of trying to call a kernel, and request it, and are put in the line. ;)

//Hm, shared memory/cache to store e.g. neuron fired etc.


//So, a couple options. 1 is to use like, hyper-Q, or the MPS thing from NVIDIA. But that seems to require running daemons on each thing, not dynamic. I'd rather have it automatically find the guys.

//Seems best option for me is to use openMP or multiple threads in a single "MPI" rank on each machine, with access to the processor. Of course this only
//I can have certain "requirements" like requires GPU for this task, etc.
//Problem is that if user program runs via system(), it must handle everything itself. Which is extremely inefficient? And I'd have to specify the GPU,
//and it would be differnet processes for sure. Hm, in that case the MPS solution might be best... requires starting it.

//Anyway, make everything "CUDA AWARE", so that each worker thread queries and gets its own GPU at the beginning. Instead of using multiple ranks,
//use multiple threads within the thing, via openMP? Question is how I would specify which thread groups get which threads basically...
//Even if I ignore openMP, I have thread support in openMP, and I pass it saying "start N threads". This will be much more robust than openMP, but
//it means that if user e.g. wants to start CPU openMP computations, it's different. That will depend on user though I guess. Each MPI "process" is
//replaced with System. Seems like forking will fork() the whole process but control will only be from the calling thread. Not what I want ;)
//Great, so imagine I've set up the MPS thing, so I can call from each MPI process. Hm. I only have 24 on there haha... Yea I definitely need to
//swap out the GPUs and memory... I think (theoretically), everything is transparent to the user, it's just ranks calling. But for each, I need to
//tell which GPU to use. So I need to know how many to do per GPU, in which case I can compute from my index which GPU idx I should be in. And do
//GPU probing the normal way.

//OK cudaSetDevice is per-thread, so we are good (note API is state dependent...). But state is per-thread, so we good. Thank you NVIDIA.
//Um, so start MPI server, set which guys are visible (at first?). I still need to do specify in some way how many will be on each GPU... After which
//I can enumerate normally...

//Note making everything visible in PSWEEP is probably better, I.e. let per-worker guys be smarter about querying GPUs.

//can specify (simplest) number per GPU. Or, some other things, like amount of memory etc...and then it will divide it up based on that...

//NOTE GPU are asynch? We can start from same guy...calling different programs...user functs...ugh.

//Um, we don't really care about CPU as we assume GPU number is the bottleneck. So, we have a scheduler of GPU...? Just make each user program run in
//single thread, and...communicate? I.e. I have a single guy, with some # of threads, and each guy runs on those threads I guess...
//Basically, in each user code (?), instead of calling simply "kernel", they call something else? Nah, they each have their own stream in the GPU, great.
//I start one for each GPU. I basically set device at beginning, and then run user program normally (as I get it). But it's a single worker. So, the
//worker can hold up to some number of guys... i.e. it can be overscheduled.

//But, only if user selects that option. Just have "worker groups", that communicate to each toerh rofl. Nah.
//Just have the thing I imagined, where it sends work to worker, and worker accepts up to some number of guys if it has "room". Master keeps track
//of how many each guy can have (and whether it is multi-threaded...?)
//meanwhile, within it there are the threads, each of which has its own "stream" on the GPU, and uses it if it wants.

//We want to (possibly) give more threads to the process though...for e.g. set up. I.e. use all available CPUs heh. Best way to do that is make a single
//MPI process, which uses all available processes...?

//Great, problem is still that the MPI process needs to handle communicating with the main guy via MPI commands such as SEND and RECEIVE. Is that "safe"?

//"tag" is second. In receive. Same with send. Just an int.

//OK, no jokes. Changes are:
//1) worker-thread now spawns N std::thread, which each loop independently
//2) Threads have directly indices which are the tags
//3) From the point of view of master, it only sends to "workers", which are then evaluated down to RANK/TAG. I have the mapping for that.
//I need to select my "GPU DEVICE" if possible beforehand (if there are any). Assume nothing about user thing, select it based on what, #SM or smthing.
//Like, if CUDA is defined, it makes/creates those guys and sets DEV to that. That's all haha. In other word, master makes the N ranks, which each
//in turn creates the threads based on some other parameter. That is fine. Problem is that workers don't directly map to ranks anymore. They now map
//to rank by: worker# is 1+(rank-1)*NPERRANK. So, 0th rank is nothing ofc. ??? rank of worker is (worker# - TAG)/NPERRANK + 1. tag is worker# % NPERRANK.



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
size_t compute_gpu_idx( const size_t& localidx, const size_t& nworkersperrank, const size_t& ranknum )
{
  size_t mydevidx = localidx / nworkersperrank;
  std::string devname = "Tesla K80";

#ifdef CUDA_SUPPORT
  std::vector<size_t> devs = findlegaldevices_byname(devname); //Base it on some requirements of user.
  if( devs.size() <= mydevidx )
    {
      fprintf(stderr, "compute_gpu_idx: ERROR, not enough GPU devices on host (I am [%ld] but there are only [%ld])! (Rank [%ld])\n", mydevidx, devs.size(), ranknum);
      exit(1);
    }
  mydevidx = devs[mydevidx];
#endif

  return mydevidx;
}

void set_cuda_device(const size_t& idx)
{

#ifdef CUDA_SUPPORT
  set_cuda_device(idx);
#endif

}
