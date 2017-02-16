//Single entry point for using psweep2
//Include this header, and call run_search with correct options.
//Many options specify a "config" file name.
//REV: Does user ever need to access the global name list? Or do I add
//it myself?

//Also gives options to manipulate results of search (e.g. subsample
//history of HDF5 file, etc.)

#pragma once

#include <searcher.h>

class psweep2
{
 public:
  
  void run_search( const int& argc, char* *argv );
  
  void register_funct( const std::string& name, const fake_system_funct_t& funct);

    
 private:
  
  searcher srch;
  
}; //end class psweep2


//User needs to be able to query the correct "dev" via script accesses.
//For example get_worker_number, etc. will get number of worker that this is being executed under...
//This is not possible under my current way because scripts are totally evaluated to string before being farmed.
//I need a way of specifying the worker or something?
//See, normally it shouldn't "matter" which node I'm on. But, it does. So, best to have single MPI worker that farms them off to devices? Ugh...
//In the end, if DEV relies on the worker#, that is the problem I guess?

//Do it this way:
//Method to get which worker I am (will be) farmed to on this node.
//Method for user to know # of GPUs useable on that node (just by ranks), and then I simply index into that list of useable ones. E.g. if
// 0 (bad) 1 (good) 2 (bad) 3 (good), then I will put rank 0 of this node on 1, and rank 1 of this node on 3.
//However, it will take a long time to query the device (?), so actually it's better to have it hard-set per worker...
//Slow, but do it? Might use a different GPU each time though..ugh. Also, assumes # of ranks (and locale of ranks?) is the same over the whole run.
//But fine. So, need to basically leave a "hackable" variable that will be literally only overwritten when it is scheduled.
//Each worker knows its value in its own thing, so it just writes that to that variable (if it exists). We can set a bit/idx to it to make it faster.
