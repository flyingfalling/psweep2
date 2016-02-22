//REV: Advantage over MT is that the original mixing is more important (because of 100% accept rate orz).
//To do MT, we have a problem because the EPSILON-RHO is not a proper probability, in fact it goes from (-inf, EPSILON]. Which is weird.
//So we can't multiply them. We can sum/avg them of course. Also, we can't select from multinomial in MT step to choose one. We don't have a "max",
//so we can't even normalize them to a probability (since they go all the way to negative infinity...)

//Note, DREAM ABC can't be used to optimize because it requires a "target" performance that it will try to maximize? Or do we just want "better than
//the other one" or something? If we're optimizing, why stop after a set amount?

//REV: Same as DREAM-ABC, but now I draw from historical points Z, only 1/10 of Z. (i.e. draw normally from 1/10 size, then multiply those numbers by 10,
//correcting for chain chunking in X_hist matrix).

//Supposedly this solves problem of lack of mixing...if there is a pure bimodal distribution. However, in reality, it quickly converges to one of the optima without sampling the others. If we totally "lose" an optima, it is a large problem. Note that even if e.g. 9/10 chains go to OPTIM1, and 1/10 goes to
//optim2, we are still very unlikely to "jump" to optim2...but fitness is just as good. This should be solved by more noise, or by having more chains (to
//better represent the space). However, in this case convergence is much slower...?

//Problem is that we need # of chains to at least equal number of dimensions? Using some MT strategy might give us better proposals...problem is we
//can't use a "probability difference" target to select it? In other ones, we use a ratio. In this one, we use a ratio, but ratio is ALWAYS to accept if
//its > 1, in other words if PREV/REF >= 1, accept...wow.

//So, we could make MT-DREAM-ABC-z. This might actually be kind of nice because it adds "uncertainty" in there by mixing multiple "tries" in...so
//ratio will be PROPOSAL/(REFERENCE+OLD). And, by that ratio of fitnesses (?huh?) we choose whether to go... it is SUM of them...

//Because we e.g. might have one chain in one optimum location that has (slightly better) fitness, and so hits to THERE will always jump that way,
//whereas hits this way won't. I'm really leaning towards this MT, aren't I...

//REV: Need to make it so that I can read from "NAMESPACE" varlist in addition to the hierarchical one...
//Need to make way to search "list" of hierarchical varlist.

//REV: Run on SC stuff (SGI/SGS). But, it won't do e.g. normalization for us. Use PARAFAC etc., to see what patterns are for different neurons (mouse/trials).

//Difference between SGI and SGS?

//Will it extract "long" tail of it, fast decay, etc.? How about for
//CURRENT clamp vs. VOLTAGE clamp etc.,?

//How about 2PT?

//Could give us some idea about analysis.


#pragma once

#include <algorithm>    // std::find
#include <filesender.h>
#include <hdf5_collection.h>
#include <random>
#include <vector_utils.h>
#include <rand_utils.h>
#include <stat_helpers.h>
#include <idxsortIMPL.h>
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
  void new_state ( const std::string& statefilename,
		   const std::vector<std::string>& varnames,
		   const std::vector<float64_t>& mins,
		   const std::vector<float64_t>& maxes,
		   const std::vector<std::string>& observation_varnames,
		   const std::vector<float64_t>& observation_stats,
		   float64_t epsil=2.0, //0.05,
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
		   )
		   
  {
    dream_abc_state::new_state( statefilename, varnames,
				mins, maxes, observation_varnames,
				observation_stats, epsil, maxgens,
				numchains, ndelta, bnoise, bstar,
				rthresh, GRskip, nCR, pCRskip, pjump,
				backupskip );


    state.add_float64_matrix( Z_hist, varnames );
    state.add_int64_parameter( M0_param, varnames.size()*M0d_mult ); //REV: Need to fill Z with M0 at first.
    state.add_int64_parameter( K_Zthin_param, Kthin );
    state.add_int64_parameter( M_param, 0 );
  }
  
  void add_current_gen_to_Z()
  {
    std::vector<std::vector<float64_t>> Xcurr = get_current_gen();
    state.add_to_matrix<float64_t>( Z_hist, state.get_varnames(Z_hist), Xcurr );
    return;
  }
  
  
  //@OVERLOAD
  void cleanup_gen()
  {

    int64_t tgen = get_param<int64_t>(t_gen);
    int64_t Kskip = get_param<int64_t>( K_Zthin_param );
    
    if( (tgen+1) % Kskip == 0 )
      {
	add_current_gen_to_Z();
	fprintf(stdout, "ADDING ROWS TO Z!\n");
      }
    
    dream_abc_state::cleanup_gen();
  }

  
  //@OVERLOAD
  std::vector<std::vector<float64_t> > get_mypairs_vectors( const std::vector<size_t>& pairidxs )
  {
    std::vector<std::vector<float64_t> > mypairs = state.get_matrix_row_slice<float64_t>( Z_hist, pairidxs );
    return mypairs;
  }
  
  //@OVERLOAD
  std::vector<size_t> draw_DE_pairs( const size_t& npairs,
				     std::default_random_engine& rand_gen )
  {
    return choose_k_indices_from_N_no_replace( get_param<int64_t>( M_param ), npairs*2, rand_gen );
  }

  //@OVERLOAD
  //Now draw and add to Z, draw M0 from the hypercube?  BUt init pop is same.
  void generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg )
  {

    START_GEN();

    std::vector< std::vector< float64_t> > Zsamples = latin_hypercube( get_vector_param<float64_t>(dim_mins_param),
									get_vector_param<float64_t>(dim_maxes_param),
									get_param<int64_t>(M0_param),
									rand_gen );  //prior_function();

    state.add_to_matrix<float64_t>( Z_hist, state.get_varnames(Z_hist), Zsamples);
    set_param<int64_t>( M_param, Zsamples.size() );
    
    //Use latin hypercube? or just N Uniform?
    std::vector< std::vector <float64_t> > samples = latin_hypercube( get_vector_param<float64_t>(dim_mins_param),
								      get_vector_param<float64_t>(dim_maxes_param),
								      get_param<int64_t>(N_chains_param),
								      rand_gen );  //prior_function();
    
    //Add these guys to X as well since it is the first generation. Computing generation will fill H, piH, etc.
    
    state.add_to_matrix<float64_t>( X_hist, state.get_varnames( X_hist ), samples );
    
    compute_generation_fitnesses( samples, fs, pg );
    
    //fprintf(stdout, "DREAM-ABC: MASTER: Finished: Computed fitnesses!\n");    
    //piH_hist and rho_hist now contain the most recent chain computations!
    
    //Fill piX manually since we won't run an ACCEPT step here.
    std::vector<std::vector<float64_t> > Hfit = state.get_last_n_rows<float64_t>( piH_hist, get_param<int64_t>(N_chains_param) );
    for(size_t c=0; c<Hfit.size(); ++c)
      {
	state.add_row_to_matrix( piX_hist, Hfit[c] );
      }

    init_population_mean_var();
    
    //fprintf(stdout, "FINISHED INIT POP\n");
    
    END_GEN();
  } //end generate_init_pop
  
  //CTOR
  //REV: **DO NOT CALL PARENT CONSTRUCTOR IT WILL BREAK THINGS B/C BASE CLASS FUNCTIONS CALLING OVERRIDDEN FUNCTIONS WILL NO LONGER CALL
  //THE OVERRIDDEN VERSION!!!! **
  dream_abc_z_state()
    {
      initialize();
    }
  
  dream_abc_z_state( const long& seed)
    {
      initialize(seed);
    }
  
};


//REV: First implement MT-DREAM-Z
void search_dream_abc_z( const std::string& statefilename,
			 const std::vector<std::string>& varnames,
			 const std::vector<float64_t>& mins,
			 const std::vector<float64_t>& maxes,
			 const std::vector<std::string>& observed_varnames,
			 const std::vector<float64_t>& observed_data,
			 parampoint_generator& pg,
			 filesender& fs
			 )
{
  dream_abc_z_state state;

  //Should I load or not?
  state.new_state( statefilename,
		   varnames,
		   mins,
		   maxes,
		   observed_varnames,
		   observed_data
		   );
  
  state.run( fs, pg );

  return;
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




