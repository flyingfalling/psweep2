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
  
  void run_search( const int& argc, char* *argv )
  {
    //REV: Crap, all ranks are parsing the options...
    optlist opts( argc, argv );
    srch.run_search( opts );
    return;
   
  }
  
  void register_funct( const std::string& name, const fake_system_funct_t& funct)
  {
    srch.register_funct( name, funct );
  }
  
 private:
  
  searcher srch;
  
}; //end class psweep2
