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
  D( nCR_param );
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
    state.add_int64_parameter( nCR_param, nCRpairs );
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
    state.add_float64_matrix( X_hist, varnames );
    state.add_float64_matrix( piX_hist, state.dummy_colnames(1));
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

    std::vector< std::vector< float64_t > > firstpop = N_uniform<float64_t>( get_vector<float64_t>( dim_mins_param),
									     get_vector<float64_t>( dims_max_param),
									     get_param<int64_t>(N_chains_param),
									     rand_gen );
    
    std::vector<varlist<std::string> > vls( get_param<int64_t>(N_chains_param ) );
    //Make varlist from names and doubles...
    for(size_t x=0; x<firstpop.size(); ++x)
      {
	varlist<std::string> vl;
	vl.make_varlist<float64_t>(  names,  vals );
	vls[x] = vl;
      }
    //Compute the fitnesses of them
    fs.comp_pp_list( pg, vls );
    
    //TODO REV: get results from RESULTS, specifically FITNESS? Just get from RESULTS (should be a varlist heh)
    //We know same order as varlists we gave, so OK.

    std::vector<varlist<std::string> > results = pg.get_last_N_results( get_param<int64_t>(N_chains_param) );
    
    pg.cleanup_parampoints_upto( get_param<int64_t>(N_chains_param) );
    
    set_param<int64_t>( M,
			( get_param<int64_t>(M0_param) + get_param(N_chains_param) )
			);

    std::vector<float64_t> fitnesses( results.size(), 0.0 );
    //TODO: Append results to piX and piH
    for(size_t x=0; x<results.size(); ++x)
      {
	fitnesses[x] = results[x].get_float64( "FITNESS" );
	
	//The fitnesses are organized as column vectors...so I need to add "rows" one at a time lol...
	add_row_to_matrix<float64_t>( piX_hist, fitnesses[x] );
	add_row_to_matrix<float64_t>( piH_hist, fitnesses[x] );
      }

    //Note these are also the current (initial) "state"
    
  } //end generate_init_pop

  
  
};


//REV: I need to know what the EPSILON is. I also need to know how to compute P. Just compute P is max of all the guys? I.e. individual points? They are
//some kind of 2D data or something? Literal distance...? Problem is, how do I know the "correct" value of each...? In that case, fitness is
//simply rho, which is "worst". I'd like to keep track of all the values...which requires me knowing the "correct" values in this code...
//Do I really want to do the statistics in here? Just have user give me the "correct" values for each (of the variables)
//User needs to somehow pass me the "true means" here...not true means but just true values. I could force user to print them all out for some reason?
//Anyway, um, easiest to just not record them at all? Or at least store them somehow? For other guy, I pass params "min and max", which is just
//number of dimensions. For this one, I also need (want?) to pass a (possibly pre-computed?) set of "correct" answers?
//Meh, just have user thing return "target" and "data" guys...haha. Ugh, would need to parse them then...
//If user program is going to compute them and return them anyways, what is downside of forcing user to pass them at start time? he must
//pre-compute them separately, that is the issue. But it's a one-time thing, so that's fine.
struct dream_abc_state
{
  hdf5_collection state;
  
  D( t_gen );
  //D( M );
  D( pCR_state );
  D( DeltaCR_state );
  //D( CR_used_state );
  //D( CR_cnts_state );
  D( GR_vals_state );
  D( pdelta_state );
  //D( delta_used_state );
  //D( mt_CR_used_state );
  //D( mt_delta_used_state );
  
  
  //D( Z_hist );
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

  D( epsilon_param );
  D( Y_param );
  D( rho_hist ); //History of individual errors per data point... Could run N trials each...but meh. Note this is subtracted from epsilon already
  D( observation_dims_param );
  
  D( T_max_gens_param );
  D( N_chains_param );
  D( d_dims_param );
  D( n_delta_param );
  //D( M0_param );
  //D( K_thin_param );
  //D( k_param );
  D( b_noise_param );
  D( bstar_param );
  D( R_thresh_param );
  D( GR_skip_param );
  D( nCR_param );
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
		  const std::vector<std::string>& observation_varnames,
		  const std::vector<float64_t>& observation_stats,
		  float64_t epsil=0.025,
		  int64_t maxgens=100000,
		  int64_t numchains=10,
		  int64_t ndelta=1,
		  float64_t bnoise=0.05,
		  float64_t bstar=1e-6,
		  float64_t rthresh=1.2,
		  int64_t GRskip=10,
		  int64_t nCR=3,
		  int64_t pCRskip=10,
		  float64_t pjump=0.1
		  )
  {
    //REV: Would like to name these...?
    state.add_float64_vector( Y_param, observation_varnames, observation_stats );

    state.add_float64_matrix( rho_hist, observation_varnames );

    state.add_float64_parameter( epsilon_param, epsil );
    state.add_int64_parameter( observation_dims_param, observation_varnames.size() );
    
    //Names should be stored in EACH matrix (wow!)
    state.add_float64_vector( dim_mins_param, mins );
    state.add_float64_vector( dim_maxes_param, maxes );
    
    //REV: Need to do this in order...ugh.
    state.add_int64_parameter( T_max_gens_param, maxgens );
    state.add_int64_parameter( N_chains_param, numchains );
    state.add_int64_parameter( n_delta_param, ndelta );
    state.add_int64_parameter( d_dims_param, mins.size());
    
    state.add_float64_parameter( b_noise_param, bnoise );
    state.add_float64_parameter( bstar_param, bstar );
    state.add_float64_parameter( R_thresh_param, rthresh );

    state.add_int64_parameter( GR_skip_param, GRskip );
    state.add_int64_parameter( nCR_param, nCR );
    state.add_int64_parameter( pCR_skip_param, pCRskip );

    state.add_float64_parameter( p_jump_param, pjump );
    
    state.add_int64_parameter( t_gen, 0 );
    //state.add_int64_parameter( M, 0 );
    
    state.add_float64_vector( pCR_state, std::vector<float64_t>( nCR, 1.0/nCR) );
    state.add_float64_vector( DeltaCR_state, std::vector<float64_t>( nCR, 0.0 ) );
    //state.add_int64_vector( CR_used_state, std::vector<int64_t>( nCRpairs, 1e10) );
    //state.add_int64_vector( CR_cnts_state, std::vector<int64_t>( nCRpairs, 0) );
    state.add_float64_vector( GR_vals_state, std::vector<float64_t>( mins.size(), 666.66 ) );
    state.add_float64_vector( pdelta_state, std::vector<float64_t>( ndelta, 1.0/ndelta ) );
    //state.add_int64_vector( delta_used_state, std::vector<int64_t>(ndelta,  1e10 ) );
    
    state.add_float64_matrix( Z_hist, varnames );
    state.add_float64_matrix( X_hist, varnames );
    state.add_float64_matrix( piX_hist, state.dummy_colnames(1));
    state.add_float64_matrix( H_hist, varnames );
    state.add_float64_matrix( piH_hist, state.dummy_colnames(1) );
    state.add_float64_matrix( pCR_hist, state.dummy_colnames(nCRpairs) );
    state.add_float64_matrix( DeltaCR_hist, state.dummy_colnames(nCRpairs) );

    state.add_int64_matrix( CR_used_hist, state.dummy_colnames(1)  );
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
	fprintf(stderr, "ERROR: DREAM ABC Even backup file of h5 file [%s] is not sane! Exiting\n", bufname.c_str() );
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

  void START_GEN()
  {
    state.set_param<int64_t>( PRE_GEN_ITER, state.get_param<int64_t>(PRE_GEN_ITER)+1 );
  }

  void END_GEN()
  {
    state.set_param<int64_t>( t_gen, state.get_param<int64_t>(t_gen)+1 );
    state.set_param<int64_t>( POST_GEN_ITER, state.get_param<int64_t>(POST_GEN_ITER)+1 );
  }
  
  void run(std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg )
  {
    if( get_param<int64_t>( t_gen ) == 0 )
      {
	START_GEN();
	generate_init_pop( rand_gen, fs, pg );
	END_GEN(); //updates t_gen as well.
      }
    while( get_param<int64_t>( t_gen ) < get_param<int64_t>( T_max_gens_param ) )
      {
	run_generation();
      }
  }

  run_generation()
  {
    //1) Generate proposals (including jump, choosing CR, choosing DELTA, etc.)
    START_GEN();
    
    std::vector<std::vector<float64_t> > proposals = make_proposals();
    
    //2) Compute fitness of new proposals
    compute_generation_fitnesses( proposals );

    compute_acceptance();

    move_chains();
    
    //Compute other things like GR, etc.
    //Update JUMP probabilities, CR, etc. based on USED CR indices etc.
    
    
    END_GEN();
  }

  std::vector<float64_t> make_single_proposal( const std::vector<float64_t>& parent, std::default_random_engine& rand_gen )
  {
    std::vector<float64_t> proposal = parent;

    
    std::vector<size_t> pairidxs;
    std::vector<size_t> movingdims;
    float64_t gamma;
    
    choose_moving_dims_and_npairs( pairidxs, movingdims, gamma, rand_gen );

    std::vector<std::vector<float64_t> > mypairs = state.get_matrix_row_slice<float64_t>( X_hist, pairidxs );
    
    
    std::normal_distribution<float64_t> gdist(0, get_param<float64_t>( bstar_param ) );
    float64_t bwid = get_param<float64_t>( b_noise_param  );
    std::uniform_real_distribution<float64_t> udist(-bwid, bwid);
    
    
    std::vector<std::vector<T> > dimdiffs(  mypairs.size()/2, std::vector<float64_t>(proposal.size(), 0 ) );
    std::vector<T> dimdiffs_total( proposal.size(), 0);
    std::vector<T> dimdiffs_wnoise( proposal.size(), 0);
    
    if(mypairs.size() % 2 != 0)
      {
	fprintf(stderr, "ERROR, generate_proposal, passed mypairs is size (%ld) but should be divisible by 2...\n", mypairs.size());
	exit(1);
      }
    
    size_t pairs= mypairs.size()/2;
    for(size_t d=0; d<movingdims.size(); ++d)
      {
	size_t actualdim = movingdims[d];
	    
	float64_t diff = 0;
	for(size_t p=0; p<pairs; ++p)
	  {
	    //REV: same as summing all p*2 first, then subtracting sum of all p*2+1...?
	    diff += ( mypairs[p*2][ actualdim ] - mypairs[(p*2)+1][ actualdim ] );
		
	  }
	    
	double e_noise = 1.0 + udist( rand_gen ); //+r8_uniform_sample(-b_uniform_noise_radius, b_uniform_noise_radius); //we draw these independently for each dimension, I guess.
	double epsilon_noise = gdist( rand_gen ); //r8_normal_sample(0, bstar_gaussian_noise_std);
	    
	proposal[ actualdim ] += e_noise * gamma * diff + epsilon_noise;
      }
  
    proposal = edge_handling( proposal, rand_gen );
    return proposal;
  }

  std::vector<float64_t> edge_handling( const std::vector<float64_t>& proposal, std::default_random_engine& rand_gen )
  {
    //CLAMP seems best, but will stack up if it keeps trying to go out the side
    //REFLECT will give right "distribution", but bad position...
    //Random sample seems most "fair" but it will cause it to miss important (proposal) density that would otherwise cluster near the edge.
    std::vector<float64_t> newproposal = proposal;
    std::vector<float64_t> dim_mins = get_vector<float64_t>( dim_mins_param );
    std::vector<float64_t> dim_maxes = get_vector<float64_t>( dim_maxes_param );
    std::uniform_real_distribution<float64_t> udist(0.0, 1.0);
    
    for(size_t d=0; d<proposal.size(); ++d)
      {
	if(proposal[d] > dim_maxes[d])
	  {
	    newproposal[d] = (2.0 * dim_maxes[d]) - proposal[d];
	  }
	else if(proposal[d] < dim_mins[d])
	  {
	    newproposal[d] = (2.0 * dim_mins[d]) - proposal[d];
	  }
	
	//if still, just uniform (e.g. because of excessively large / small jumps that takes it "back out the other side" )
	if(newproposal[d] > dim_maxes[d] || newproposal[d] < dim_mins[d])
	  {
	    proposal[d] = dim_mins[d] + (dim_maxes[d] - dim_mins[d]) * udist( rand_gen );
	  }
      }
    
    return newproposal;
  }
  
  std::vector<std::vector< float64_t> > make_proposals(std::default_random_engine& rand_gen)
  {
    std::vector<std::vector<float64_t> > proposals;
    std::vector<std::vector<float64_t> > Xcurr = state.get_last_n_rows<float64_t>( X_hist, get_param<int64_t>(N_chains_param) );
    for(size_t c=0; c<Xcurr.size(); ++c)
      {
	std::vector< std::vector< float64_t > > mypairs = choose_pairs( ); //this uses CR
	std::vector<float64_t> proposal = make_single_proposal( Xcurr[c], rand_gen );
		
	proposals.push_back(proposal);
      }

    return proposals;
  }
  
  float64_t incrementally_compute_mean( const float64_t& prevmean, const float64_t& newsample, const int64_t& newn )
  {
    float64_t mean = prevmean*(newn-1);
    mean = (mean+newsample)/newn;
    return mean;
  }

  //https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
  float64_t incrementally_compute_var( const float64_t& prevmean, const float64_t& prevvar, const float64_t& newmean, const float64_t& newsample, const int64_t& newn, float64_t& prevM2n )
  {
    float64_t M2n = prevM2n + (newsample - prevmean) * (newsample - newmean);
    float64_t newvar = M2n / (float64_t)(newn-1);
    //float64_t div = (newsample - prevmean);
    //float64_t newvar = (n-2)/(n-1)*prevvar + (1/n)*(div*div);
    return newvar;
  }
  
  std::vector<float64_t> compute_std_for_all_dims_all_history()
  {
    //Keep a "partial sum" updated, and just do the change in N.
    //Also, keep a "deviation from mean" for each. Problem is, change in mean will cause change in the deviation...

    //http://math.stackexchange.com/questions/102978/incremental-computation-of-standard-deviation
    
    std::vector<float64_t> prev_stds = get_prev_stds();
    std::vector<float64_t> prev_means = get_prev_means();
    std::vector<float64_t> prev_M2ns = get_prev_M2ns();

    int64_t skippcr= get_param<int64_t>( pCR_skip_param );
    int64_t nchains= get_param<int64_t>( N_chains_param );
    std::vector< std::vector<float64_t> > newsamples = state.get_last_n_rows( X_hist, nchains );
    int64_t n = state.get_num_rows( X_hist ) - newsamples.size();
    
    for(size_t c=0; c<newsamples.size(); ++c)
      {
	for(size_t d=0; d<prev_stds.size(); ++d)
	  {
	    float64_t newsample = newsamples[c][d];
	    float64_t prevmean = prev_means[d];
	    ++n; //increment n, b/c we added a sample
	    prev_means[d] = incrementally_compute_mean( prev_means[d], newsample, n );
	    prev_stds[d] = incrementally_compute_var( prevmean, prev_stds[d], prev_means[d], newsample, n, prev_M2ns[d] );
	  }
      }

    //Write STDS, MEANS, M2NS. Note STD is sqrt of prev_stds.
    return prev_stds;
  }
  
  void update_DeltaCR()
  {
    std::vector<std::vector<float64_t> > Xlast2gens = get_last_n_rows( X_hist, 2 * get_param<int64_t>( N_chains_param ) );

    std::vector<std::vector<float64_t> > lastgen =
      std::vector<std::vector<float64_t> >( Xlast2gens.begin(), Xlast2gens.begin()+get_param<int64_t>( N_chains_param ) );
    
    std::vector<std::vector<float64_t> > thisgen =
      std::vector<std::vector<float64_t> >( Xlast2gens.begin() + get_param<int64_t>( N_chains_param ), Xlast2gens.end() );

    std::vector<std::vector<int64_t> > usedCRidxs = state.get_last_n_rows( CR_used_hist, get_param<int64_t>( N_chains_param ) );

    std::vector<float64_t> DeltaCR = state.get_last_n_rows( DeltaCR_hist, 1 );
    
    if(thisgen.size() != lastgen.size() )
      {
	fprintf(stderr, "REV: ERROR fuxxed up iterarors in updateDELTACR\n");
	exit(1);
      }
    
    std::vector<float64_t> stds = compute_std_for_all_dims_all_history();

    //SQRT BECAUSE THEY ARE ACTUALLY VARIANCE...
    vector_sqrt<float64_t>( stds );
    
    size_t ndims = get_param<int64_t>( d_dims_param );
    //Update DeltaCR (norm squared jump distances...)
    for(size_t c=0; c<lastgen.size(); ++c)
      {
	size_t used = usedCRidxs[c][0];
	float64_t dist=0;
	for(size_t d=0; d<ndims; ++d)
	  {
	    if(stds[d] > 0 )
	      {
		double a=( lastgen[c][d] - thisgen[c][d] ) / ( stds[d] );
		dist += a*a;
	      }
	  }

	DeltaCR[d] += dist;
      }

    //Update/write DeltaCR (add row to history)
  }
  
  
  //REV: Huh, there's probably a more intelligent way to do this than manually check each one?
  std::vector<size_t> choose_moving_dims( const size_t& Didx,  std::default_random_engine& rand_gen ) 
  {
    std::uniform_real_distribution<float64_t> udist(0.0, 1.0);
    
    std::vector<size_t> mymovingdims;

    float64_t crval = (float64_t)(Didx+1) / (float64_t)get_param<int64_t>( nCR_param );
    for(size_t d=0; d < get_param<int64_t>( d_dims_param ); ++d)
      {
	if( (1.0 - crval) < udist(rand_gen) )
	  {
	    mymovingdims.push_back(d);
	  }
      }
    
    if(mymovingdims.size() == 0)
      {
	//randomly select a dim to move...
	std::uniform_int_distribution<size_t> dist(0, get_param<int64_t>( d_dims_param ) - 1);
	size_t choice = dist(rand_gen);
	mymovingdims.push_back(choice);
      }
  }
  
  void choose_moving_dims_and_npairs( std::vector<size_t>& mypairs, std::vector<size_t>& moving_dims, float64_t& gamma, std::default_random_engine& rand_gen )
  {
    size_t Didx = choose_CR_idx( rand_gen );
    size_t tauidx = draw_num_DE_pairs( rand_gen );
    float64_t gamma = compute_gamma_nonsnooker( tauidx+1, Didx+1, rand_gen );
    if( gamma == 1.0 )
      {
	Didx = get_param<int64_t>(d_dims_param) - 1; //-1 bc its idx.
	tauidx = 0;
      }

    //Each pair gets the same pair of moving dims.
    moving_dims = choose_moving_dims( Didx, rand_gen );
    mypairs = draw_DE_pairs( tauidx+1, rand_gen );
    
    //Assume we're pushing back to the correct "new" chain at the end.
    state.add_row_to_matrix<int64_t>( CR_used_hist, std::vector<int64_t>(1, Didx ));

    state.add_row_to_matrix<int64_t>( delta_used_hist, std::vector<int64_t>(1, tauidx ));
    
    //REV: Update used_CR, used_delta, etc.
    //Also compute STD, etc., and move them based on those?
    //Actually update pCR and pDELTA?
    //pDELTA is not updated (it's always equal for each num pairs..)
    //Every generation, we update DeltaCR.
    //Once every SKIP, we update pCR.
    
    return;
  }

  size_t choose_CR_index( std::default_random_engine& rand_gen )
  {
    if( get_param<int64_t>(nCR_param) > 1 )
      {
	std::vector<size_t> chosen=multinomial_sample( pCR_state, 1, rand_gen );
	return chosen[0];
      }
    else
      {
	return 0;
      }
  }
  
  //REV: dprime/D is the number of moving dimensions, tau is the number of pairs being used...
  float64_t compute_gamma_nonsnooker(const size_t& tau, const size_t& D, std::default_random_engine& rand_gen )
  {

    std::uniform_real_distribution<float64_t> udist(0.0, 1.0);
    
    if( udist(rand_gen)  < p_set_gamma_to_one )
      {
	return ( 1.0 );
      }
    if( D < 1 || tau < 1)
      {
	fprintf(stderr, "SUPER ERROR in compute_gamma_nonsnooker: tau or D < 1\n\n");
	exit(1);
      }
    
    //optim: tabulate
    //else
    return ( 2.38 / sqrt( 2.0 * (float64_t)tau * (float64_t)D ) );
  }

  //REV: Do everything. Draw num DE pairs, also
  //Note returns delta INDEX (delta pairs is INDEX+1)
  size_t draw_num_DE_pairs( std::default_random_engine& rand_gen )
  {
    //Basically draws from PDELTA, sets it, etc.
    std::vector<float64_t> pdelta = get_vector<float64_t>( pdelta_state );
    std::vector<size_t> npairs = multinomial_sample( pdelta, 1, rand_gen );
    return (npairs[0]);
  }
  
  std::vector<size_t> draw_DE_pairs( const size_t& npairs, std::default_random_engine& rand_gen )
  {
    return choose_k_indices_from_N_no_replace( get_param<int64_t>(N_chains_param), npairs*2 );
  }

  
  void compute_GR()
  {
    int64_t timepoints = get_param<int64_t>(t_gen) / 2;
    if( timepoints < 2 )
      {
	return ;
      }
    size_t ndims = get_param<float64_t>( d_dims_param );
    size_t nchains = get_param<float64_t>( N_chains_param );
    
    //Reconstruct the chain, and take only 2nd half
    
    //REV: This is a pain in the ass, because it has to be from the last
    //50%. So, it's not possible to incrementally do it I think.
    //However, I can at least incrementally compute STD as I go.

    std::vector< std::vector< float64_t> > each_chain_and_dim_means;
    std::vector< std::vector< float64_t> > each_chain_and_dim_vars;
    
    //for each chain:
    for(size_t c=0; c<nchains; ++c)
      {
	std::vector<float64_t> means;
	std::vector<float64_t> stds;

	//for each dim
	for(size_t d=0; d<ndims; ++d)
	  {
	    float64_t newmean = incrementally_compute_mean( );
	    float64_t newstd = incrementally_compute_var( );
	  }
	
	each_chain_and_dim_means.push_back(means);
	each_chain_and_dim_vars.push_back(stds);
      }
    
    std::vector<float64_t> variance_between_chain_means(ndims, 0);
    std::vector<float64_t> means(ndims, 0);
    
    for(size_t c=0; c<nchains; ++c)
      {
	//for each dim:
	for(size_t d=0; d<ndims; ++d)
	  {
	    means[c] += each_chain_and_dim_means[c][d];
	  }
      }
    vector_divide_constant<float64_t>( means, (float64_t)nchains );
    for(size_t c=0; c<nchains; ++c)
      {
	for(size_t d=0; d<ndims; ++d)
	  {
	    float64_t tmp = (each_chain_and_dim_means[c][d] - means[d]);
	    variance_between_chain_means[d] += (tmp*tmp);
	  }
      }
    
    vector_divide_constant<T>(variance_between_chain_means, (float64_t)(nchains-1));
    if(timepoints > 1)
      {
	vector_multiply_constant<float64_t>(variance_between_chain_means, (float64_t)timepoints);
      }


    //What is the variance between chains
    std::vector<float64_t> mean_variance_all_chain_dim(ndims, 0);
    //4) Compute mean of within-sequence variances. Mean of each dim from step 3. Call W.
    for(size_t c=0; c<N_num_chains; ++c)
      {
	for(size_t d=0; d<d_num_dims; ++d)
	  {
	    mean_variance_all_chain_dim[d] += each_chain_and_dim_vars[c][d];
	  }
      }
    vector_divide_constant<float64_t>( mean_variance_all_chain_dim, (float64_t)nchains );

    std::vector<float64_t> Rstat(d_num_dims, 0);
    
    bool wouldconverge = true;
    for(size_t d=0; d<ndims; ++d)
      {
	if(mean_variance_all_chain_dim[d] > 0)
	  {
	    Rstat[d] = (float64_t)(timepoints-1)/(float64_t)timepoints + (float64_t)(nchains+1)/(float64_t)(nchains*timepoints) * ((float64_t)variance_between_chain_means[d] / (float64_t)mean_variance_all_chain_dim[d]);
	    Rstat[d] = sqrt(Rstat[d]);
	  }
	else
	  {
	    Rstat[d] = 6666.0;
	  }
	
	if( Rstat[d] >= get_param<float64_t>( R_thresh_param ) )
	  {
	    //At least one dim has R value > threshold
	    wouldconverge = false;
	  }
      }
    
  } //end compute_GR
  
  void update_pCR()
  {
    std::vector<float64_t> DeltaCR = state.get_last_n_rows<float64_t>( DeltaCR_hist, 1 );
    std::vector<int64_t> CRcnts = state.get_last_n_rows<int64_t>( CR_cnts_hist, 1 );

    std::vector<float64_t> pCR = state.get_last_n_rows<float64_t>( pCR_hist, 1 );
    double sumDeltas= vector_sum<double>(DeltaCR);
    size_t nchains = get_param<int64_t>( N_chains_param );
    size_t ncr = get_param<size_t>( nCR_param );

    bool doit=true;
    size_t tgen=get_param<int64_t>( t_gen );
    for(size_t cridx=0; cridx<ncr; ++cridx)
      {
	//just numbers I picked out of my ass
	if( CRcnts[cridx] < 50 || tgen < 1000 )
	  {
	    doit = false;
	  }
      }
    if( doit == true )
      {
	for(size_t cridx=0; cridx<nCR; ++cridx)
	  {
	    pCR[cridx] = nchains * (DeltaCR[cridx] / ((float64_t)CRcnts[cridx]));
	    pCR[cridx] /= (sumDeltas);
	  }
      }

    
    //REV: Guarantees we don't get any zero-size guys.
    //This is for numerical stability.
    float64_t sum=vector_sum<float64_t>( pCR );
    vector_divide_constant<float64_t>(pCR, sum); //+0.0001);
    
    float64_t minsize=1e-5;
    float64_t sizeper = minsize / pCR.size();
    vector_add_constant<float64_t>(pCR, sizeper);
    
    sum=vector_sum<float64_t>( pCR );
    vector_divide_constant<float64_t>(pCR, sum+minsize); //+0.0001);
    
    
    //TODO Write pCR
    state.add_row_to_matrix<float64_t>( pCR_hist, pCR );
  }

  
  
  
  void generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg )
  {

    //Use latin hypercube? or just N Uniform?
    std::vector< std::vector <float64_t> > samples = latin_hypercube(get_vector<float64_t>(dim_mins_param),
								     get_vector<float64_t>(dim_maxes_param),
								     get_param<int64_t>(M0_param),
								     rand_gen ); //prior_function();

    //Add these guys to X as well since it is the first generation. Computing generation will fill H, piH, etc.
    //
    add_to_matrix( X_hist, get_varnames( X_hist ), vals );
    
    compute_generation( get_varnames( dim_mins_param ), samples );
    
    //piH_hist and rho_hist now contain the most recent chain computations!
    
    //Fill piX manually since we won't run an ACCEPT step here.
    std::vector<std::vector<float64_t> > Hfit = state.get_last_n_rows<float64_t>( piH_hist, get_param<int64_t>(N_chains_param) );
    for(size_t c=0; c<Hfit.size(); ++c)
      {
	state.add_row_to_matrix( piX_hist, Hfit[c] );
      }
    
  } //end generate_init_pop


  std::vector<float64_t> compute_stat_divergence( const varlist<std::string>& results, std::vector<float64_t>& resultvals )
  {
    std::vector<float64_t> targ = get_vector_param<float64_t>( Y_param );
    std::vector<std::string> names = get_varnames( Y_param );
    
    resultvals.resize( targ.size() );
    
    std::vector<float64_t> ret;
    
    for(size_t x=0; x<targ.size(); ++x)
      {
	float64_t resval = results.get_float64( names[x] );
	float64_t divergence = targ[x] - resval; //REV: Will take abs val later I guess...
	ret[x] = divergence;
	resultvals[x] = resval;
      }
    
    return ret;
  }

  //Just takes absolute value of them ahaha. Sqr and then Sqrt. Waste.
  std::vector<float64_t> compute_stat_rho( const std::vector<float64_t>& divergence )
  {
    std::vector<float64_t> ret( divergence );
    vector_sq<float64_t>( ret );
    vector_sqrt<float64_t>( ret );
    return ret;
  }

  std::vector<float64_t> compute_epsilon_divergence( const std::vector<float64_t>& abs_divergence )
  {
    std::vector<float64_t> ret( abs_divergence );
    vector_subtract<float64_t>( ret, get_param<float64_t>( epsilon_param ) );
    return ret;
  }

  float64_t compute_fitness( const std::vector<float64_t>& epsilon_divergence )
  {
    return vector_min<float64_t>( epsilon_divergence );
  }

  //Automatically operates on H_hist and piH_hist. We don't check this
  //for first generation. Literally checks the piH of last N_chains_param,
  //against the piX of the last N_chains_param.
  //Ratio is computed as:
  //If piH > 0, ACCEPT,
  //else if piH < 0 && piH >= piX, ACCEPT (i.e. always accept UP if piH > 0,
  //   i.e. if it is a performant model...)
  //else, REJECT
  
  std::vector<bool> compute_acceptance( )
  {
    std::vector< std::vector< float64_t> > Hfits = state.get_last_n_rows( piH_hist, get_param<int64_t>(N_chains_param) );
    std::vector< std::vector< float64_t> > Xfits = state.get_last_n_rows( piX_hist, get_param<int64_t>(N_chains_param) );
    
    std::vector<int64_t> accept( get_param<int64_t>(N_chains_param), 0 );
    for( size_t c=0; c<Hfits.size(); ++c )
      {
	if( Hfits[c][0] >= 0  )
	  {
	    accept[c] = 1; //TRUE
	  }
	else
	  {
	    if( Hfits[c][0] >= Xfits[c][0]) 
	      {
		accept[c] = 1; //TRUE
	      }
	    else
	      {
		accept[c] = 0; //FALSE
	      }
	  }

	state.add_row_to_matrix<int64_t>( accept_hist, std::vector<int64_t>(1, accept[c]) );
      }
    
  }

  void move_chains()
  {
    //Uses accept_hist to move X to H.

    std::vector<std::vector<float64_t> > Xcurr = state.get_last_n_rows<float64_t>( X_hist, get_param<int64_t>(N_chains_param) );
    std::vector<std::vector<float64_t> > Hcurr = state.get_last_n_rows<float64_t>( H_hist, get_param<int64_t>(N_chains_param) );
    std::vector<std::vector<int64_t> > accepted = state.get_last_n_rows<int64_t>( accept_hist, get_param<int64_t>(N_chains_param) );
    
    //Get fitnesses too and use those...
    std::vector<std::vector<float64_t> > Xfit = state.get_last_n_rows<float64_t>( piX_hist, get_param<int64_t>(N_chains_param) );
    std::vector<std::vector<float64_t> > Hfit = state.get_last_n_rows<float64_t>( piH_hist, get_param<int64_t>(N_chains_param) );
    
    for( size_t c=0; c<Xcurr.size(); ++c )
      {
	if(accepted[c][0] == 1)
	  {
	    state.add_row_to_matrix( X_hist, Hcurr[c] );
	    state.add_row_to_matrix( piX_hist, Hfit[c] );
	  }
	else
	  {
	    state.add_row_to_matrix( X_hist, Xcurr[c] );
	    state.add_row_to_matrix( piX_hist, Xfit[c] );
	  }
      }
  }
  

  void compute_generation_fitnesses(const std::vector<std::vector<float64_t>>& vals  )
  {
    const std::vector<std::string> names = get_varnames( dim_mins_param );
    
    std::vector<varlist<std::string> > vls( vals.size() );
    //Make varlist from names and doubles...
    for(size_t x=0; x<firstpop.size(); ++x)
      {
	varlist<std::string> vl;
	vl.make_varlist<float64_t>(names, vals );
	vls[x] = vl;
      }
    //Compute the fitnesses of them
    fs.comp_pp_list( pg, vls );
    
    //TODO REV: get results from RESULTS, specifically FITNESS? Just get from RESULTS (should be a varlist heh)
    //We know same order as varlists we gave, so OK.

    //std::vector<varlist<std::string> > results = pg.get_last_N_results( get_param<int64_t>(N_chains_param) );
    std::vector<varlist<std::string> > results = pg.get_last_N_results( vals.size() );
    
    pg.cleanup_parampoints_upto( get_param<int64_t>( vals.size() ) );
    
    //Add these guys to H.
    add_to_matrix( H_hist, get_varnames( H_hist ), vals );
    
    std::vector<float64_t> fitnesses( results.size(), -66666.22222 );
    
    //TODO: Append results to piX and piH
    for(size_t x=0; x<results.size(); ++x)
      {
	std::vector<float64_t> statdiv;
	std::vector<float64_t> ed = compute_epsilon_divergence( compute_stat_rho( compute_stat_divergence( results[x], statdiv) ) );
	float64_t fit = compute_fitness( ed );
	fitnesses[x] = fit;
	
	//The fitnesses are organized as column vectors...so I need to add "rows" one at a time lol...
	add_row_to_matrix<float64_t>( piH_hist, fit );
	add_row_to_matrix<float64_t>( rho_hist, statdiv );
      }

    //Fitness hist is only added after we check accept/not accept...
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
