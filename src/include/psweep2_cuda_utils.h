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
