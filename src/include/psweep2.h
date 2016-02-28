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
    optlist opts( argc, argv );
    srch.run_search( opts );
    return;
    //srch = searcher( vl ); //initializes some required state using
    //parse of options. However, note this may be bubbled further in
    //and used by many other structs during the search, e.g. by
    //individual search algorithms, etc.
    //Only *they* know how to handle certain options.
  }
  
  
  
 private:

  searcher srch;
  
}; //end class psweep2
