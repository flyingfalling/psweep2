//REV: Need to make it so that I can read from "NAMESPACE" varlist in addition to the hierarchical one...
//Need to make way to search "list" of hierarchical varlist.

//REV: Run on SC stuff (SGI/SGS). But, it won't do e.g. normalization for us. Use PARAFAC etc., to see what patterns are for different neurons (mouse/trials).

//Difference between SGI and SGS?

//Will it extract "long" tail of it, fast decay, etc.? How about for
//CURRENT clamp vs. VOLTAGE clamp etc.,?

//How about 2PT?

//Could give us some idea about analysis.


#pragma once


#include <filesender.h>


//REV: start with a flat prior I guess...

//REV: pass other specific variables, like numgens etc.
//Need a way to do all of it.


//REV: Would like to save all these things to HD5 file so we can restart at any time given this STATE and the PARAMS below.
//Can do this, need to have int type and double type only, and store them as kind of "VAR" "VAL" type thing... Ugliest is to store them as strings haha.
//REV: I will include a MATRIX of guys of history, and current guys, which will just be access functions I guess.

//So much easier to save all VARIABLES and PARAMS together, although state is reset, params are not if we want to e.g. run the same one over again.

//REV: Need a way of storing not only DOUBLE matrices, but also LONG INT (and STRING? Nah...)

typedef double float64_t;
typedef long int int64_t;

#define D(s) const std::string s = #s
//#define DD(s) state.add_

struct mt_dream_z_state
{
  hdf5_collection state;
  
  D( t_gen );
  D( M );
  D( pCR_state );
  D( DeltaCR_state );
  D( CR_used_state );
  D( CR_cnts_state );
  D( GR_vals_state );
  D( pdelta_state );
  D( delta_used_state );
  D( mt_CR_used_state );
  D( mt_delta_used_state );

  
  D( Z_hist );
  D( Xall_hist );
  D( piXall_hist );
  D( H_hist );
  D( piH_hist );
  D( pCR_hist );
  D( DeltaCR_hist );
  D( CR_used_hist );
  D( CR_cnts_hist );
  D( GR_vals_hist );
  D( accept_hist );

  D( T_max_gens_param );
  D( N_chains_param );
  D( d_dims_param );
  D( n_delta_param );
  D( M0_param );
  D( K_thin_param );
  D( k_param );
  D( b_noise_param );
  D( bstar_param );
  D( R_thresh_param );
  D( GR_skip_param );
  D( nCR_pairs_param );
  D( pCR_skip_param );
  D( p_jump_param );

  D( dim_names_param );
  D( dim_mins_param );
  D( dim_maxes_param );

  D( PRE_GEN_ITER );
  D( POST_GEN_ITER );
  
  void new_state( const std::vector<std::string>& varnames,
		  const std::vector<float64_t>& mins,
		  const std::vector<float64_t>& maxes,
		  int64_t maxgens=100000,
		  int64_t numchains=10,
		  int64_t ndelta=1,
		  int64_t M0mult=100,
		  int64_t Kthin=10,
		  int64_t k=5,
		  float64_t bnoise=0.05,
		  float64_t bstar=1e-6,
		  float64_t rthresh=1.2,
		  int64_t GRskip=10,
		  int64_t nCRpairs=3,
		  int64_t pCRskip=10,
		  float64_t pjump=0.1
		  )
  {
    
    
    //Names should be stored in EACH matrix (wow!)
    state.add_float64_vector( dim_mins_param, mins );
    state.add_float64_vector( dim_maxes_param, maxes );
    
    //REV: Need to do this in order...ugh.
    state.add_int64_parameter( T_max_gens_param, maxgens );
    state.add_int64_parameter( N_chains_param, numchains );
    state.add_int64_parameter( n_delta_param, ndelta );
    state.add_int64_parameter( d_dims_param, mins.size());
    state.add_int64_parameter( M0_param, M0mult*mins.size());
    state.add_int64_parameter( K_thin_param, Kthin );
    state.add_int64_parameter( k_param, k );

    state.add_float64_parameter( b_noise_param, bnoise );
    state.add_float64_parameter( bstar_param, bstar );
    state.add_float64_parameter( R_thresh_param, rthresh );

    state.add_int64_parameter( GR_skip_param, GRskip );
    state.add_int64_parameter( nCR_pairs_param, nCRpairs );
    state.add_int64_parameter( pCR_skip_param, pCRskip );

    state.add_float64_parameter( p_jump_param, pjump );
    
    state.add_int64_parameter( t_gen, 0 );
    state.add_int64_parameter( M, 0 );
    
    state.add_float64_vector( pCR_state, std::vector<float64_t>( nCRpairs, 1.0/nCRpairs) );
    state.add_float64_vector( DeltaCR_state, std::vector<float64_t>( nCRpairs, 0.0 ) );
    state.add_int64_vector( CR_used_state, std::vector<int64_t>( nCRpairs, 1e10) );
    state.add_int64_vector( CR_cnts_state, std::vector<int64_t>( nCRpairs, 0) );
    state.add_float64_vector( GR_vals_state, std::vector<float64_t>( mins.size(), 666.66 ) );
    state.add_float64_vector( pdelta_state, std::vector<float64_t>( ndelta, 1.0/ndelta ) );
    state.add_int64_vector( delta_used_state, std::vector<int64_t>(ndelta,  1e10 ) );
    state.add_int64_matrix( mt_CR_used_state, state.dummy_colnames(numchains) );
    state.add_int64_matrix( mt_delta_used_state, state.dummy_colnames(numchains)  );
    
    state.add_float64_matrix( Z_hist, varnames );
    state.add_float64_matrix( Xall_hist, varnames );
    state.add_float64_matrix( piXall_hist, state.dummy_colnames(1));
    state.add_float64_matrix( H_hist, varnames );
    state.add_float64_matrix( piH_hist, state.dummy_colnames(1) );
    state.add_float64_matrix( pCR_hist, state.dummy_colnames(nCRpairs) );
    state.add_float64_matrix( DeltaCR_hist, state.dummy_colnames(nCRpairs) );

    state.add_int64_matrix( CR_used_hist, state.dummy_colnames(nCRpairs)  );
    state.add_int64_matrix( CR_cnts_hist, state.dummy_colnames(nCRpairs)  );
    state.add_float64_matrix( GR_vals_hist, varnames );
    state.add_int64_matrix( accept_hist, state.dummy_colnames(1) ); //REV: Which chains accepted...? Ignores MT.

    state.add_int64_parameter( PRE_GEN_ITER, 0 );
    state.add_int64_parameter( POST_GEN_ITER, 0 );
    
  }

  
  
  void load_state( const std::string& file )
  {
    state.load_collection( file );

    //Check HDF5 file is "sane", i.e. that all matrices have appropriate length given t_current_gen etc. If not, we will try the equivalent BU file.
    bool sane = issane( );
    
    if(sane == false && file.c_str()[0] != '_' && file.c_str()[1] != '_' )
      {
	std::string bufname = "__" + file;
	load( bufname, params );
      }
    else
      {
	fprintf(stderr, "Even backup file of h5 file [%s] is not sane! Exiting\n", bufname.c_str() );
	exit(1);
      }
    
  }

  bool issane( )
  {
    //REV: need to check lengths of everything and all parameters to make sure they line up...pain in the ass haha.
    //Screw it for now just go.
    return true;
  }
  
  
  //REV: These convenience functions might be better coded in the collection struct. Can be used for multiple things.
  template <typename T>
  T get_param( const std::string& s)
  {
    return state.get_numeric_parameter( s );
  }

  template <typename T>
  void set_param( const std::string& s, const T& val)
  {
    state.set_numeric_parameter( s, val );
  }

  template <typename T>
  T get_vparam( const std::string& s, const size_t& idx )
  {
    //TODO
  }
  
  template <typename T>
  void set_vparam( const std::string& s, const size_t& idx, const T& val )
  {
    //TODO
  }

  template <typename T>
  std::vector<T> get_vector( const std::string& s )
  {
    //TODO
  }

  template <typename T>
  void set_vector( const std::string& s, const std::vector<T>& val )
  {
    //TODO
  }

  //Just use raw add to matrix...do everything first raw, then wrap it?


  void generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg )
  {
    std::vector<float64_t> tmp( get_param<int64_t>( d_dims_param ) );
    std::vector< std::vector <float64_t> > toeval( N_chains_param,  std::vector<float64_t>( d_dims_param ) );

    std::vector< std::vector <float64_t> > samples = latin_hypercube(get_vector<float64_t>(dim_mins_param),
								     get_vector<float64_t>(dim_maxes),
								     get_param<int64_t>(M0_param),
								     rand_gen ); //prior_function();

    state.add_to_matrix( Z_hist, state.dummy_colnames( N_chains_param ), samples );

    std::vector< std::vector< float64_t > > firstpop = N_uniform<float64_t>( get_vector( dim_mins_param),
								  get_vector( dims_max_param),
								  get_param(N_chains_param),
								  rand_gen );

    //Make varlist from names and doubles...
    varlist<std::string> vl;
    vl.make_varlist<float64_t>( const std::vector<std::string>& names, const std::vector<float64_t>& vals );
    
    //Compute the fitnesses of them
    fs.comp_pp_list( pg, vl );

    //TODO REV: get results from RESULTS, specifically FITNESS? Just get from RESULTS (should be a varlist heh)
    
    set_param<int64_t>( M,
			( get_param<int64_t>(M0_param) + get_param(N_chains_param) )
			);

    //TODO: Append results to piXall and piH

  }
  
  
};

struct mt_dream_z
{
  mt_dream_z_state state;

  //REV: This will set/get all the guys...hehe.
  
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
