

#pragma once

#include <algorithm>    // std::find
#include <filesender.h>
#include <hdf5_collection.h>
#include <random>
#include <vector_utils.h>
#include <rand_utils.h>
#include <stat_helpers.h>
#include <idxsort.h>
#include <dream_abc.h>
//REV: start with a flat prior I guess...

//REV: pass other specific variables, like numgens etc.
//Need a way to do all of it.


//REV: Would like to save all these things to HD5 file so we can restart at any time given this STATE and the PARAMS below.
//Can do this, need to have int type and double type only, and store them as kind of "VAR" "VAL" type thing... Ugliest is to store them as strings haha.
//REV: I will include a MATRIX of guys of history, and current guys, which will just be access functions I guess.

//So much easier to save all VARIABLES and PARAMS together, although state is reset, params are not if we want to e.g. run the same one over again.

//REV: Need a way of storing not only DOUBLE matrices, but also LONG INT (and STRING? Nah...)

struct dream_abc_z_state : public dream_abc_state
{
  //All of DREAM_ABC guys, plus:
  D( Z_hist );
  D( M0_param );
  D( K_Zthin_param );
  D( M_param );


  //@OVERLOAD
  inline void new_state ( const std::string& statefilename,
		   const std::vector<std::string>& varnames,
		   const std::vector<float64_t>& mins,
		   const std::vector<float64_t>& maxes,
		   const std::vector<std::string>& observation_varnames,
		   const std::vector<float64_t>& observation_stats,
		   const std::vector<float64_t>& observation_epsilons,
		   int64_t maxgens=1e5,
		   int64_t numchains=100,
		   int64_t ndelta=3,
		   float64_t bnoise=0.05,
		   float64_t bstar=1e-6,
		   float64_t rthresh=1.2,
		   int64_t GRskip=50,
		   int64_t nCR=3,
		   int64_t pCRskip=10,
		   float64_t pjump=0.05,
		   int64_t backupskip=10,
		   int64_t M0d_mult=100,
		   int64_t Kthin=5
		   );
		  
  
  inline void add_current_gen_to_Z();
  
  
  //@OVERLOAD
  inline void cleanup_gen();

  
  //@OVERLOAD
  inline std::vector<std::vector<float64_t> > get_mypairs_vectors( const std::vector<size_t>& pairidxs );
  
  //@OVERLOAD
  inline std::vector<size_t> draw_DE_pairs( const size_t& npairs,
				     std::default_random_engine& rand_gen );

  //@OVERLOAD
  //Now draw and add to Z, draw M0 from the hypercube?  BUt init pop is same.
  inline void generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg );
  
  struct abczconfig
  {
    //OPTIONS
    bool restart=false;
    std::string _statefilename="__ERROR_NOSTATEFILENAME";
    std::string _varfilename="__ERROR";
    std::string _obsfilename="__ERROR";
    std::string _epsilonsfilename="__ERROR";

    std::vector<std::string> _varnames;
    std::vector<float64_t> _mins;
    std::vector<float64_t> _maxes;

    std::vector<std::string> _observation_varnames;
    std::vector<float64_t> _observation_stats;
    
    std::vector<float64_t> _observation_epsilons;
    
    //Optional (only loads params for sweep, i.e. doesnt load vars etc.)
    std::string _CONFIGFILE="__OPTIONALCONFIG";
    bool configexists=false;
    
    //And, finally other values/options.
    //float64_t _epsil=2.0;
    int64_t _maxgens=1e5;
    int64_t _numchains=100;
    int64_t _ndelta=3;
    float64_t _bnoise=0.05;
    float64_t _bstar=1e-6;
    float64_t _rthresh=1.2;
    int64_t _GRskip=50;
    int64_t _nCR=3;
    int64_t _pCRskip=10;
    float64_t _pjump=0.05;
    float64_t _backupskip=10;
    int64_t _M0d_mult=100;
    int64_t _Kthin=5;

    inline void load_abcz_params( const std::string& fname )
    {
      //haha, global access to my dudes...oh well.
      //Easier to just make them named same thing, and search for them there (i.e. in side the alredy created state?)
      //So, instead of passing, just pass a varlist? These are default params here I guess...ugh.
      fprintf(stdout, "REV: WARNING: LOAD_ABCZ_PARAMS from config file [%s] is not implemented yet!!!! Sorry. Ideas: best is to simply treat it as a varlist, and do matching against param string names on other (state) side. In which case NEWSTATE is not passed actual arguments at all, but a varlist, and we can parse them as float64_t or int64_t or whatever we want.\n", fname.c_str());
      return;
    }
    
    
    
  }; //end internal struct abcconfig


  //OVERLOADED -- NO **MASKED*** i.e. different return type!!!
  //REV: UGLY UGLY UGLY PARSE SHARED STUFF BEFORE, NEED TO MAKE THIS BETTER LATER!!
  inline abczconfig parseopts( optlist& opts );
  
    
    


  
  
  //OVERLOADED? No, actually a different function...
  inline void create_with_config( abczconfig& c );

  //CTOR
  //REV: **DO NOT CALL PARENT CONSTRUCTOR IT WILL BREAK THINGS B/C BASE CLASS FUNCTIONS CALLING OVERRIDDEN FUNCTIONS WILL NO LONGER CALL
  //THE OVERRIDDEN VERSION!!!! **
  
  inline dream_abc_z_state( optlist& opts );

  inline dream_abc_z_state();
  
  inline dream_abc_z_state( const long& seed);
  
};


inline void search_dream_abc_z( optlist& opts,
			 parampoint_generator& pg,
			 filesender& fs
			 );

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////////////////////////////////////////////////// OLD VERSION


//REV: First implement MT-DREAM-Z
inline void search_dream_abc_z( const std::string& statefilename,
			 const std::vector<std::string>& varnames,
			 const std::vector<float64_t>& mins,
			 const std::vector<float64_t>& maxes,
			 const std::vector<std::string>& observed_varnames,
			 const std::vector<float64_t>& observed_data,
			 parampoint_generator& pg,
			 filesender& fs
			 );


//Methods to:
// generate random numbers...
// binomial/multinomial
// take slices of vectors?
// save to vectors (file)?
// better way to handle generations than manually computing with N+k*x etc...
// compute std, mean, etc., for computing rubin-gelman statistic.

//Want to divorce it from the growing_matrix thing? Or keep up with it haha. Just use HDF5 I guess...

// Ability to modify things like ngenerations etc. partway through?


//REV: for the other thing, basically I pass as arguments a "memfile" holder thing, and user program must read from that.
//I always check THAT THING FIRST for files, THEN I check the real FS.
//Problem is when WRITING files, I always need to make sure whether I should or shouldn't write the file (actually) to disk.
//If it's a data file, keep it around, but figure that out later heh.

//Furthermore, I have to "register" a function (in a struct?) or something, and it takes those memfile thngs as arg.

//This way I can do everything fine.

//Write to HDF5 when? Copy back all files? Store things in what way? In a single HDF5 is easiest...need to "read" from there then (keep it in memory?)

//REV: With HDF5, keep "data" (out full memory arrays) in memory, and then only write every generation? So at read-time, I will actually be reading.
//Note, the things I will actually need access to will depend on the search algo. For DREAM-z, we use history, so we need Z history, and current members.
//I think we need X for estimating convergence too? For estimating posterior, we use X (full history?)

//For others we need nothing or etc.

//So, after each time step, we abort partway through updating HDF5 file, what happens? Something messed up.
//Write out to the file as we go, but when read, we need to make sure we're reading something? Hm, just read from the freaking file. We keep the
//file in "memory" (i.e. a reference to it). We need to keep refreshing it.
//Anyway, seems way to do it is with "chunking"...we will still read from files but at least it will chunk it. Problem is that it won't keep a large
//part in the cache? But we need to make sure to read enough at a time?

//Give some "convenience" functions to read into/out of HDF5 file.

//Make examples. I basically make "tables" (or variables), and I write them to HDF5 file as "state" type things. I have single variables sometimes.
//REV: It might be best to also separate the "state" variable from the "history" variable (like, files, individual results files, etc.)
//Otherwise I'll have to move it around. OK, I like that I guess...
//Have a "record" file, that includes what, all data files etc. too? And config like stuff?

//Anyway, hold each "variable", which will ahve blah. We must know it based on "type" of sweep, how to separate "generations etc." etc.

//each one will be extendible...etc.? Do it like "groups"? Inside. No, just name datatypes in /BASE? Easiest that way...




#include <dream_abc_z.cpp>
