
#pragma once


#include <filesender.h>


//REV: start with a flat prior I guess...

//REV: pass other specific variables, like numgens etc.
//Need a way to do all of it.


//REV: Would like to save all these things to HD5 file so we can restart at any time given this STATE and the PARAMS below.
//Can do this, need to have int type and double type only, and store them as kind of "VAR" "VAL" type thing... Ugliest is to store them as strings haha.
//REV: I will include a MATRIX of guys of history, and current guys, which will just be access functions I guess.

//So much easier to save all VARIABLES and PARAMS together, although state is reset, params are not if we want to e.g. run the same one over again.
struct mt_dream_z_state
{
  size_t t_current_gen;
  size_t M_size_Z;
  
  std::vector<float64_t> pCR_CR_probs;
  std::vector<float64_t> DeltaCR_norm_sq_jumpdists;
  std::vector<size_t> CR_used_indices;
  std::vector<size_t> CR_counts;
  std::vector<size_t> GR_values;
  
  std::vector<float64_t> pdelta_delta_probs;
  std::vector<size_t> delta_used_probs;

  std::vector<std::vector<size_t>> mt_CR_used_indices;
  std::vector<std::vector<size_t>> mt_delta_used_indices;
};

struct mt_dream_z_params
{
  size_t T_max_gens;
  size_t N_num_chains;
  size_t d_num_dims;
  size_t n_delta;
  size_t M0_start_size_Z;
  size_t K_thinning_factor_Z;
  size_t k_num_mt;
  float64_t b_uniform_noise_radius;
  float64_t bstar_gauss_noise_std;
  float64_t R_GR_thresh;
  size_t GR_skip_gens;
  size_t nCR_num_pairs;
  size_t pCR_skip_gens;
  size_t p_jump;
  std::vector<std::string> dim_names;
  std::vector<float64_t> dim_mins;
  std::vector<float64_t> dim_mins;
};

struct mt_dream_z
{
  mt_dream_z_params params;
  mt_dream_z_state state;

  void run()
  {
    // 1 compute initial population M0

    // 2 while not finished with max generations, compute the next generation
    
    
  }
  
};


void search_mt_dream_z( const std::vector<std::string>& varnames,
			const std::vector<double>& mins,
			const std::vector<double>& maxes,
			parampoint_generator& pg,
			filesender& fs )
{
  
  
}


//REV: First implement MT-DREAM-Z
void search_dream_abc( const std::vector<std::string>& varnames,
		       const std::vector<double>& mins,
		       const std::vector<double>& maxes,
		       parampoint_generator& pg,
		       filesender& fs )
{
  
  
  
  
  
  
}


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




struct dream_abc_searcher
{
  //Has some local variables to define parameters of search
  //and some other guys to determine current state of search.
  
  
  //At any rate we need to do generations.
  
  dream_abc_searcher()
  {
    
  }
  
  
}; //end dream_abc_searcher
