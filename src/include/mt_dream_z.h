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




void search_mt_dream_z( const std::vector<std::string>& varnames,
			const std::vector<double>& mins,
			const std::vector<double>& maxes,
			parampoint_generator& pg,
			filesender& fs )
{
  
  
}

