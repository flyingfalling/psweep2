
//REV: Is epsilon value only single value for all observed parameters?
//Observed parameters may be on totally different scales, which causes
//a problem. We would like to define some kind of "percent" parameter, so
//that we can appropriately scale them all to same level?

//For example, one is measure mV, another is measuring pA, another is
//measuring avg spike rate. Note, in all cases, it represents the
//*divergence* from the target. Is 50 mV from 15 mV worse/better than
// 100 pA from 80 pA? 35 vs 20 error...
//If we specify a normalization method it is better?
//I.e. all model values are normalized...



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
//REV: start with a flat prior I guess...

//REV: pass other specific variables, like numgens etc.
//Need a way to do all of it.


//REV: Would like to save all these things to HD5 file so we can restart at any time given this STATE and the PARAMS below.
//Can do this, need to have int type and double type only, and store them as kind of "VAR" "VAL" type thing... Ugliest is to store them as strings haha.
//REV: I will include a MATRIX of guys of history, and current guys, which will just be access functions I guess.

//So much easier to save all VARIABLES and PARAMS together, although state is reset, params are not if we want to e.g. run the same one over again.

//REV: Need a way of storing not only DOUBLE matrices, but also LONG INT (and STRING? Nah...)

#include <commontypes.h>

#define D(s) const std::string s = #s
//#define DD(s) state.add_



struct dream_abc_state
{
  hdf5_collection state;
  std::default_random_engine rg;
  seedgen sg;
  
  D( t_gen );

  D( X_hist );
  D( piX_hist );
  D( H_hist );
  D( piH_hist );
  D( pCR_hist );
  D( DeltaCR_hist );
  D( CR_used_hist );
  D( CR_cnts_hist );
  D( GR_vals_hist );
  D( accept_hist );
  D( GR_hist );

  D( pdelta_param );
  
  D( epsilon_param ); //REV: Changed. Epsilon is now 1xM vector.
  D( Y_param );
  D( model_observ_diverg_hist ); //History of individual errors per data point... Could run N trials each...but meh. Note this is subtracted from epsilon already
  D( observation_dims_param );
  
  D( T_max_gens_param );
  D( N_chains_param );
  D( d_dims_param );
  D( n_delta_param );
  D( b_noise_param );
  D( bstar_param );
  D( GR_thresh_param );
  D( GR_skip_param );
  D( nCR_param );
  D( pCR_skip_param );
  D( p_jump_param );
  D( backup_regularity_param );
  
  D( dim_names_param );
  D( dim_mins_param );
  D( dim_maxes_param );

  //used for computing incrementally mean/variance of X for pCR update.
  D( X_variance_hist );
  D( X_mean_hist );
  D( X_M2n_hist );
  
  //Used for sanity (if we exit partway through a generation, these will be not equal).
  D( PRE_GEN_ITER );
  D( POST_GEN_ITER );
  
  virtual void new_state(const std::string& statefilename,
			 const std::vector<std::string>& varnames,
			 const std::vector<float64_t>& mins,
			 const std::vector<float64_t>& maxes,
			 const std::vector<std::string>& observation_varnames,
			 const std::vector<float64_t>& observation_stats,
			 const std::vector<float64_t>& observation_epsilons,
			 int64_t maxgens=1e5,
			 int64_t numchains=50,
			 int64_t ndelta=3,
			 float64_t bnoise=0.05,
			 float64_t bstar=1e-6,
			 float64_t rthresh=1.2,
			 int64_t GRskip=50,
			 int64_t nCR=3,
			 int64_t pCRskip=10,
			 float64_t pjump=0.05,
			 float64_t backupskip=10
			 );

  
  
  void load_state( const std::string& file, const bool& bu );
  
  bool issane( );
  
  
  //REV: These convenience functions might be better coded in the collection struct. Can be used for multiple things.
  template <typename T>
  T get_param( const std::string& s);

  template <typename T>
  void set_param( const std::string& s, const T& val);
  
  template <typename T>
  std::vector<T> get_vector_param( const std::string& s );

  //Just use raw add to matrix...do everything first raw, then wrap it?

  void START_GEN();

  void END_GEN();
  
  void run( filesender& fs, parampoint_generator& pg );

  void run_generation(filesender& fs, parampoint_generator& pg);

  virtual void backup();
    
  virtual void cleanup_gen();
  //REV: Better to make a "pure virtual" and not derive from dream_abc...but some shared one?
  virtual std::vector<std::vector<float64_t> > get_mypairs_vectors( const std::vector<size_t>& pairidxs );
  
  std::vector<float64_t> make_single_proposal( const size_t& mychainidx,
					       const std::vector<std::vector<float64_t>>& Xcurr,
					       std::default_random_engine& rand_gen );

  std::vector<float64_t> edge_handling( const std::vector<float64_t>& proposal,
					const std::string& boundstype,
					std::default_random_engine& rand_gen );

  
  std::vector<std::vector<float64_t> > get_current_gen();
  
  std::vector<std::vector< float64_t> > make_proposals(std::default_random_engine& rand_gen);

  void update_CRcnts();
  
  float64_t incrementally_compute_mean( const float64_t& prevmean, const float64_t& newsample, const int64_t& newn );

  //https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
  float64_t incrementally_compute_var( const float64_t& prevmean,
				       const float64_t& prevvar,
				       const float64_t& newmean,
				       const float64_t& newsample,
				       const int64_t& newn,
				       float64_t& prevM2n );
  
  std::vector<float64_t> compute_variance_for_all_dims_all_history();

  //REV: Whoops, I need to update DeltaCR for the USED INDICES!!!!
  void update_DeltaCR();
  
  
  //REV: Huh, there's probably a more intelligent way to do this than manually check each one?
  std::vector<size_t> choose_moving_dims( const size_t& Didx,
					  std::default_random_engine& rand_gen ) ;
  
  void choose_moving_dims_and_npairs( std::vector<size_t>& mypairs,
				      std::vector<size_t>& moving_dims,
				      float64_t& gamma,
				      std::default_random_engine& rand_gen );

  size_t choose_CR_index( std::default_random_engine& rand_gen );
  
  //REV: dprime/D is the number of moving dimensions, tau is the number of pairs being used...
  float64_t compute_gamma_nonsnooker(const size_t& tau,
				     const size_t& D,
				     std::default_random_engine& rand_gen );

  //REV: Do everything. Draw num DE pairs, also
  //Note returns delta INDEX (delta pairs is INDEX+1)
  size_t draw_num_DE_pairs( std::default_random_engine& rand_gen );
  
  virtual std::vector<size_t> draw_DE_pairs( const size_t& npairs,
					     std::default_random_engine& rand_gen );

  
  bool compute_GR();
  
  void update_pCR();

  
  
  
  virtual void generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg );


  //We don't compute DeltaCR etc., but we need to populate the history of the state variables for incremental computation
  //of DeltaCR
  void init_population_mean_var();

  std::vector<float64_t> compute_stat_divergence( varlist<std::string>& results, std::vector<float64_t>& resultvals );

  //Just takes absolute value of them ahaha. Sqr and then Sqrt. Waste.
  //Absolute value of distance...
  std::vector<float64_t> compute_stat_abs( const std::vector<float64_t>& divergence );

  std::vector<float64_t> compute_epsilon_divergence( const std::vector<float64_t>& abs_divergence );

  float64_t compute_rho( const std::vector<float64_t>& epsilon_divergence );
  
  void compute_acceptance();

  void move_chains();
  

  void compute_generation_fitnesses( const std::vector<std::vector<float64_t>>& vals, filesender& fs, parampoint_generator& pg  );


  void init_random()

  void init_random(const long& seed);

  void initialize( );

  void initialize(const long& seed );

  //Are these "loaded" at load time? NO!!! They aren't. We don't want these dirty state variables around...
  //Well, statefilename might be fine, as varfilename, etc.
  
  

  struct abcconfig
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

    void load_abc_params( const std::string& fname )
    {
      //haha, global access to my dudes...oh well.
      //Easier to just make them named same thing, and search for them there (i.e. in side the alredy created state?)
      //So, instead of passing, just pass a varlist? These are default params here I guess...ugh.
      fprintf(stdout, "REV: WARNING: LOAD_ABC_PARAMS from config file [%s] is not implemented yet!!!! Sorry. Ideas: best is to simply treat it as a varlist, and do matching against param string names on other (state) side. In which case NEWSTATE is not passed actual arguments at all, but a varlist, and we can parse them as float64_t or int64_t or whatever we want.\n", fname.c_str());
      return;
    }
    
    
    
  }; //end internal struct abcconfig

  //REV: UGLY UGLY UGLY WIll be MASKED By derived..(for now)
  void create_with_config( abcconfig& c );

  //REV: UGLY UGLY will be MASKED by derived!!! (for now)
  //If we actually modify things "in-line" that might be better.
  //But user creation function might have a different view of things.
  //But, other than this, everything is default I assume?
  abcconfig parseopts( optlist& opts );

  
  void parse_dreamabcfile( const std::string& fname, std::vector<std::string>& _varnames, std::vector<float64_t>& _mins, std::vector<float64_t>& _maxes );


  void parse_obsdatafile( const std::string& observfname, std::vector<std::string>& _observation_varnames, std::vector<float64_t>& _observation_stats );

  void parse_epsilondatafile( const std::string& epsilfname, const std::vector<std::string>& _varnames, std::vector<float64_t>& _epsilons );
  
  //CTOR
  dream_abc_state( optlist& opts );
  
  //CTOR
  dream_abc_state();

  dream_abc_state( const long& seed);


  //Sample from X_hist etc.

  void enumerate_to_file( const std::string& matname, const std::string& fname, const size_t& thinrate, const size_t& startpoint );
  
  //Note, do we only want to do it after convergence?
  void enumerate_X_GR_fitness( const std::string& dir, const size_t& genskiprate=10, const size_t& startgen=0 );
      
};




void search_dream_abc( optlist& opts,
		       parampoint_generator& pg,
		       filesender& fs
		       );



/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////////////////////////////////////////////////// OLD VERSION



//REV: First implement MT-DREAM-Z
void search_dream_abc( const std::string& statefilename,
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




