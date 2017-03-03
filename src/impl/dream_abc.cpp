#include <dream_abc.h>




 void dream_abc_state::new_state(const std::string& statefilename,
			 const std::vector<std::string>& varnames,
			 const std::vector<float64_t>& mins,
			 const std::vector<float64_t>& maxes,
			 const std::vector<std::string>& observation_varnames,
			 const std::vector<float64_t>& observation_stats,
			 const std::vector<float64_t>& observation_epsilons,
					int64_t maxgens,
					int64_t numchains,
					int64_t ndelta,
					float64_t bnoise,
					float64_t bstar,
					float64_t rthresh,
					int64_t GRskip,
					int64_t nCR,
					int64_t pCRskip,
					float64_t pjump,
			 float64_t backupskip
			 )
  {
    state.new_collection( statefilename );
    
    //Observations of data from real world, that we will fit.
    state.add_float64_vector( Y_param, observation_varnames, observation_stats );
    
    state.add_float64_matrix( model_observ_diverg_hist, observation_varnames );

    //state.add_float64_parameter( epsilon_param, epsil );
    state.add_float64_vector( epsilon_param, observation_varnames, observation_epsilons );
    state.add_int64_parameter( observation_dims_param, observation_varnames.size() );
    
    //Names should be stored in EACH matrix (wow!)
    state.add_float64_vector( dim_mins_param, varnames, mins );
    state.add_float64_vector( dim_maxes_param, varnames, maxes );

    state.add_float64_vector( pdelta_param, std::vector<float64_t>(ndelta, (1.0/(float64_t)ndelta)) );
    
    //REV: Need to do this in order...ugh.
    state.add_int64_parameter( T_max_gens_param, maxgens );
    state.add_int64_parameter( N_chains_param, numchains );
    state.add_int64_parameter( n_delta_param, ndelta );
    state.add_int64_parameter( d_dims_param, mins.size());
    
    state.add_float64_parameter( b_noise_param, bnoise );
    state.add_float64_parameter( bstar_param, bstar );
    state.add_float64_parameter( GR_thresh_param, rthresh );

    state.add_int64_parameter( GR_skip_param, GRskip );
    state.add_int64_parameter( nCR_param, nCR );
    state.add_int64_parameter( pCR_skip_param, pCRskip );

    state.add_float64_parameter( p_jump_param, pjump );
    
    state.add_int64_parameter( t_gen, 0 );

    state.add_int64_parameter( backup_regularity_param,  backupskip );
    
    state.add_float64_matrix( X_hist, varnames );
    state.add_float64_matrix( piX_hist, 1);
    state.add_float64_matrix( H_hist, varnames );
    state.add_float64_matrix( piH_hist, 1 );
    state.add_float64_matrix( pCR_hist, nCR );
    //Initialize pCR history too?
    state.add_row_to_matrix( pCR_hist, std::vector<float64_t>( nCR, 1.0/nCR ) );
    
    state.add_float64_matrix( DeltaCR_hist, nCR );
    state.add_row_to_matrix<float64_t>( DeltaCR_hist, std::vector<float64_t>( nCR, 0.0 ) );
    
    state.add_float64_matrix( GR_hist, varnames );

    state.add_int64_matrix( CR_used_hist, 1  );
    state.add_int64_matrix( CR_cnts_hist, nCR  );
    state.add_row_to_matrix<int64_t>( CR_cnts_hist, std::vector<int64_t>( nCR, 0 )  ); //counts start off at zero...of course...
    
    state.add_float64_matrix( GR_vals_hist, varnames );
    state.add_int64_matrix( accept_hist, 1 );

    state.add_float64_matrix( X_variance_hist, varnames );
    state.add_float64_matrix( X_mean_hist, varnames );
    state.add_float64_matrix( X_M2n_hist, varnames );
    
    //REV: Not clear how to do this. Obviously, the first mean is just the first one. The first variance is 0 (undefined? If it's /(n-1)). The first
    //M2n is likewise undefined?
    
    state.add_int64_parameter( PRE_GEN_ITER, 0 );
    state.add_int64_parameter( POST_GEN_ITER, 0 );
    
  }

  
  
  void dream_abc_state::load_state( const std::string& file, const bool& bu )
  {
    state.load_collection( file );

    fprintf(stdout, "Finished loading HDF5 collection [%s]\n", file.c_str() );
    //Check HDF5 file is "sane", i.e. that all matrices have appropriate length given t_current_gen etc. If not, we will try the equivalent BU file.
    bool sane = issane( );

    if(sane)
      {
	return;
      }
    else if(sane == false && bu == false )
      {
	
	std::string file_name;
	std::string file_path = get_canonical_dir_of_fname( file, file_name );
	std::string bufname = file_path + "/" + "__" + file_name;
	if( file[0] == '/')
	  {
	    bufname = "/" + bufname;
	  }
	
	//std::string bufname = "__" + file;
	//state.load_collection( bufname );
	//state = hdf5_collection; //"undo" load?
	state.clear();
	//REV: BETTER! Copy it to the non __ file.
	fprintf(stdout, "Copying file [%s] to [%s]\n", bufname.c_str(), file.c_str());
	copy_file( bufname, file );

	bool already_tried_backup=true;
	load_state( file, already_tried_backup );
      }
    else
      {
	fprintf(stderr, "ERROR: DREAM ABC Even backup file of h5 file [%s] is not sane! Exiting\n", file.c_str() );
	exit(1);
      }
    
  }

  bool dream_abc_state::issane( )
  {
    //REV: need to check lengths of everything and all parameters to make sure they line up...pain in the ass haha.
    //Screw it for now just go.
    if( get_param<int64_t>(PRE_GEN_ITER) == get_param<int64_t>(POST_GEN_ITER))
      {
	return true;
      }
    else
      {
	fprintf(stderr, "WARNING: LOAD hdf5 collection: file that was loaded is NOT sane. Will automatically attempt to load backup...\n");
	return false;
      }
  }
  
  
  //REV: These convenience functions might be better coded in the collection struct. Can be used for multiple things.
  template <typename T>
  T dream_abc_state::get_param( const std::string& s)
  {
    return state.get_numeric_parameter<T>( s );
  }

  template <typename T>
  void dream_abc_state::set_param( const std::string& s, const T& val)
  {
    state.set_numeric_parameter<T>( s, val );
  }
  
  template <typename T>
  std::vector<T> dream_abc_state::get_vector_param( const std::string& s )
  {
    return state.get_last_row<T>( s );
  }

  //Just use raw add to matrix...do everything first raw, then wrap it?

  void dream_abc_state::START_GEN()
  {
    set_param<int64_t>( PRE_GEN_ITER, get_param<int64_t>(PRE_GEN_ITER)+1 );
  }

  void dream_abc_state::END_GEN()
  {
    set_param<int64_t>( t_gen, get_param<int64_t>(t_gen)+1 );
    set_param<int64_t>( POST_GEN_ITER, get_param<int64_t>(POST_GEN_ITER)+1 );
  }
  
  void dream_abc_state::run( filesender& fs, parampoint_generator& pg )
  {
    if( get_param<int64_t>( t_gen ) == 0 )
      {
	generate_init_pop( rg, fs, pg );
      }
    
    int64_t maxgens = get_param<int64_t>( T_max_gens_param );
    while( get_param<int64_t>( t_gen ) < maxgens )
      {
	run_generation(fs, pg);
	//Backup every 10th generation?
      }
  }

  void dream_abc_state::run_generation(filesender& fs, parampoint_generator& pg)
  {
    //1) Generate proposals (including jump, choosing CR, choosing DELTA, etc.)
    START_GEN();


    //fprintf(stdout, "RUN GENERATION [%ld]: about to make proposals\n", get_param<int64_t>( t_gen ));
    std::vector<std::vector<float64_t> > proposals = make_proposals( rg );
    //fprintf(stdout, "RUN GENERATION [%ld]: FINISHED to make proposals. Will compute fitnesses...\n", get_param<int64_t>( t_gen ));
    
    //2) Compute fitness of new proposals
    compute_generation_fitnesses( proposals, fs, pg );
    //fprintf(stdout, "RUN GENERATION [%ld]: FINISHED compute fitnesses...\n", get_param<int64_t>( t_gen ));
    
    compute_acceptance();
    //fprintf(stdout, "RUN GENERATION [%ld]: FINISHED compute acceptances...\n", get_param<int64_t>( t_gen ));
    

    
    move_chains();
    //fprintf(stdout, "RUN GENERATION [%ld], FINISHED move chains!\n", get_param<int64_t>(t_gen));
    
    //Compute other things like GR, etc.
    //Update JUMP probabilities, CR, etc. based on USED CR indices etc.
    update_DeltaCR();
    


    cleanup_gen();
    
        
    END_GEN();

    backup();
  }

  void dream_abc_state::backup()
  {
    int64_t tgen = get_param<int64_t>(t_gen);
    int64_t backupskip = get_param<int64_t>(backup_regularity_param);
    if( (tgen+1) % backupskip == 0 )
      {
	state.backup();
      }
    
  }
  
   void dream_abc_state::cleanup_gen()
  {
    int64_t tgen = get_param<int64_t>(t_gen);
    int64_t crskip = get_param<int64_t>(pCR_skip_param );
    int64_t grskip = get_param<int64_t>(GR_skip_param );
    
    
    if( (tgen+1) % crskip == 0 )
      {
	update_pCR();
      }
    
    if( (tgen+1) % grskip == 0)
      {
	bool wouldconverge = compute_GR();
      }
    
  }

  //REV: Better to make a "pure virtual" and not derive from dream_abc...but some shared one?
  std::vector<std::vector<float64_t> > dream_abc_state::get_mypairs_vectors( const std::vector<size_t>& pairidxs )
  {
    //For normal dream ABC, get current gen
    std::vector<std::vector<float64_t>> Xcurr = get_current_gen();
    std::vector<std::vector<float64_t> > mypairs = indices_to_vector_slices< std::vector<float64_t> >(Xcurr, pairidxs );
    return mypairs;
    
  }
  
  std::vector<float64_t> dream_abc_state::make_single_proposal( const size_t& mychainidx,
					       const std::vector<std::vector<float64_t>>& Xcurr,
					       std::default_random_engine& rand_gen )
  {

    std::vector<float64_t> parent = Xcurr[mychainidx];
    std::vector<float64_t> proposal = parent;
    
    
    std::vector<size_t> pairidxs;
    std::vector<size_t> movingdims;
    float64_t gamma;

    choose_moving_dims_and_npairs( pairidxs, movingdims, gamma, rand_gen );

    //fprintf(stdout, "Pair idxs: ");
    //print1dvec_row<size_t>( pairidxs );
    
    std::vector<std::vector<float64_t> > mypairs = get_mypairs_vectors( pairidxs );
    
        
    std::normal_distribution<float64_t> gdist(0, get_param<float64_t>( bstar_param ) );
    float64_t bwid = get_param<float64_t>( b_noise_param  );
    std::uniform_real_distribution<float64_t> udist(-bwid, bwid);
    
    std::vector< std::vector<float64_t> > dimdiffs(  mypairs.size()/2, std::vector<float64_t>(proposal.size(), 0 ) );
    std::vector<float64_t> dimdiffs_total( proposal.size(), 0);
    std::vector<float64_t> dimdiffs_wnoise( proposal.size(), 0);

    
    if(mypairs.size() % 2 != 0)
      {
	fprintf(stderr, "ERROR, generate_proposal, passed mypairs is size (%ld) but should be divisible by 2...\n", mypairs.size());
	exit(1);
      }
    
    size_t pairs= mypairs.size()/2;
    if(mypairs.size() < 2)
      {
	fprintf(stderr, "ERROR, mypairs size < 2 [%ld]\n", mypairs.size());
	exit(1);
      }
    for(size_t d=0; d<movingdims.size(); ++d)
      {
	size_t actualdim = movingdims[d];
	
	float64_t diff = 0;
	for(size_t p=0; p<pairs; ++p)
	  {
	    //REV: same as summing all p*2 first, then subtracting sum of all p*2+1...?
	    if( mypairs.size() <= p*2+1 )
	      {
		fprintf(stderr, "REV: Error, mypairs size < p*2+1 (=[%ld], vs [%ld])\n", mypairs.size(), p*2+1);
		exit(1);
		
	      }
	    if( mypairs[p*2].size() <= actualdim )
	      {
		fprintf(stderr, "REV: Error, mypairs p*2 size < actualdim, [%ld] vs [%ld]\n", mypairs[p*2].size(), actualdim );
		exit(1);
	      }

	    if( mypairs[p*2+1].size() <= actualdim )
	      {
		fprintf(stderr, "REV: Error, mypairs p*2+1 size < actualdim, [%ld] vs [%ld]\n", mypairs[p*2+1].size(), actualdim );
		exit(1);
	      }
	    
	    diff +=
	      ( mypairs[p*2][ actualdim ] -
		mypairs[(p*2)+1][ actualdim ] );
		
	  }
	    
	double e_noise = 1.0 + udist( rand_gen ); //+r8_uniform_sample(-b_uniform_noise_radius, b_uniform_noise_radius); //we draw these independently for each dimension, I guess.
	double epsilon_noise = gdist( rand_gen ); //r8_normal_sample(0, bstar_gaussian_noise_std);

	//if(proposal.size() <= movingdims[d] )
	if(proposal.size() <= actualdim )
	  {
	    fprintf(stderr, "Error, moving dim is outside of proposal size!\n");
	    exit(1);
	  }
	
	proposal[ actualdim ] += e_noise * gamma * diff + epsilon_noise;
      }
    
    
    fprintf(stdout, "Chain [%ld] (gamma==[%lf]):\n", mychainidx, gamma);
    print1dvec_row<float64_t>( parent );
    print1dvec_row<float64_t>( proposal );
    
  
    proposal = edge_handling( proposal, "FOLD", rand_gen );
    
    
    return proposal;
  }

  std::vector<float64_t> dream_abc_state::edge_handling( const std::vector<float64_t>& proposal,
					const std::string& boundstype,
					std::default_random_engine& rand_gen )
  {
    //CLAMP seems best, but will stack up if it keeps trying to go out the side
    //REFLECT will give right "distribution", but bad position...
    //Random sample seems most "fair" but it will cause it to miss important (proposal) density that would otherwise cluster near the edge.
    std::vector<float64_t> newproposal = proposal;
    std::vector<float64_t> dim_mins = get_vector_param<float64_t>( dim_mins_param );
    std::vector<float64_t> dim_maxes = get_vector_param<float64_t>( dim_maxes_param );
    std::uniform_real_distribution<float64_t> udist(0.0, 1.0);

    //REV: Could "Clamp" or just straight random sample...but clamp causes problems due to all clustering right on edge. Reflect kind of does this too
    //but better estimates "local width"? Uniform is most "fair"?

    if( boundstype.compare( "REFLECT" ) == 0 )
      {
	for(size_t d=0; d<proposal.size(); ++d)
	  {
	    if(proposal[d] > dim_maxes[d])
	      {
		newproposal[d] = (2.0 * dim_maxes[d]) - proposal[d]; //dimmax - ( proposal-dimmax ). =  dimmax - proposal + dimmax. = 2*dimmax - proposal.
	      }
	    else if(proposal[d] < dim_mins[d])
	      {
		newproposal[d] = (2.0 * dim_mins[d]) - proposal[d]; //should be: ( proposal-dimmin ). E.g. 0 - 5, it is out -5. So need to go back +5.
		//So, 2*5 = 10, minus 0 = 10. i.e. 5 inside. If mins=-5, and I'm out -10, that is -10. Minus -10 is +10 = 0, which is +5 from min. OK.
	      }
	
	    //if still, just uniform (e.g. because of excessively large / small jumps that takes it "back out the other side" )
	    if(newproposal[d] > dim_maxes[d] || newproposal[d] < dim_mins[d])
	      {
		newproposal[d] = dim_mins[d] + (dim_maxes[d] - dim_mins[d]) * udist( rand_gen );
	      }
	  }
      }
    else if( boundstype.compare( "FOLD" ) == 0 )
      {
	for(size_t d=0; d<proposal.size(); ++d)
	  {
	    if(proposal[d] > dim_maxes[d])
	      {
		newproposal[d] = proposal[d] - dim_maxes[d]; //This will give how much it is outside. Add this to dimmin.
		newproposal[d] = dim_mins[d] + newproposal[d];
	      }
	    else if(proposal[d] < dim_mins[d])
	      {
		newproposal[d] = proposal[d] - dim_mins[d]; //-5 min and -10, this will give -10 + 5 = -5. So, correct is dimmax + -5
		//If 5 min and -5 proposal, (i.e. -10 outside), will be -5 - 5 = -10. So will be -10 from maxes. OK.
		newproposal[d] = dim_maxes[d] + newproposal[d];
	      }
	
	    //if still, just uniform (e.g. because of excessively large / small jumps that takes it "back out the other side" )
	    if(newproposal[d] > dim_maxes[d] || newproposal[d] < dim_mins[d])
	      {
		newproposal[d] = dim_mins[d] + (dim_maxes[d] - dim_mins[d]) * udist( rand_gen );
	      }
	  }
      }
    
    return newproposal;
  }

  
  std::vector<std::vector<float64_t> > dream_abc_state::get_current_gen()
  {
    size_t nchains = get_param<int64_t>(N_chains_param);
    return ( state.get_last_n_rows<float64_t>( X_hist, nchains ) );
  }
  
  std::vector<std::vector< float64_t> > dream_abc_state::make_proposals(std::default_random_engine& rand_gen)
  {
    size_t nchains = get_param<int64_t>(N_chains_param);
    std::vector<std::vector<float64_t> > proposals;
    std::vector<std::vector<float64_t> > Xcurr = get_current_gen(); //state.get_last_n_rows<float64_t>( X_hist, nchains );

    //fprintf(stdout, "MAKE PROPOSALS: Got last n rows of X [%ld]\n", get_param<int64_t>(N_chains_param));
    for(size_t c=0; c<Xcurr.size(); ++c)
      {
	//fprintf(stdout, "Attempting to make proposal [%ld]\n", c);
	std::vector<float64_t> proposal = make_single_proposal( c, Xcurr, rand_gen ); //haha, could just send whole thing...and chain# Easier.
	//fprintf(stdout, "FINISHED to make proposal [%ld]\n", c);
	
	proposals.push_back(proposal);
      }

    /*for(size_t c=0; c<nchains; ++c)
      {
	fprintf(stdout, "Chain [%ld]:\n", c);
	print1dvec_row<float64_t>( Xcurr[c] );
	print1dvec_row<float64_t>( proposals[c] );
	}*/
    
    //fprintf(stdout, "Finished making all props, will update CR Cnts in make_all_proposals\n");
    update_CRcnts();
    //fprintf(stdout, "Finished update CR cnts...\n");
    return proposals;
  }

  void dream_abc_state::update_CRcnts()
  {
    //Update CR_cnts_hist based on last CR_used_hist.
    std::vector<std::vector<int64_t> > usedCRidxs = state.get_last_n_rows<int64_t>( CR_used_hist, get_param<int64_t>( N_chains_param ) );

    std::vector<int64_t> CRcnts = state.get_last_row<int64_t>( CR_cnts_hist );
    
    for(size_t x=0; x<usedCRidxs.size(); ++x)
      {
	++CRcnts[ usedCRidxs[x][0] ];
      }
    state.add_row_to_matrix<int64_t>( CR_cnts_hist, CRcnts );
    fprintf(stdout, "CR cnts: ");
    print1dvec_row<int64_t>( CRcnts );
  }
  
  float64_t dream_abc_state::incrementally_compute_mean( const float64_t& prevmean, const float64_t& newsample, const int64_t& newn )
  {
    float64_t mean = prevmean*(newn-1);
    if(newn == 0 )
      {
	fprintf(stderr, "REV ERROR: Newn is 0...div by zero\n");
	exit(1);
      }
    mean = (mean+newsample)/(float64_t)newn;
    return mean;
  }

  //https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
  float64_t dream_abc_state::incrementally_compute_var( const float64_t& prevmean,
				       const float64_t& prevvar,
				       const float64_t& newmean,
				       const float64_t& newsample,
				       const int64_t& newn,
				       float64_t& prevM2n )
  {
    float64_t M2n = prevM2n + (newsample - prevmean) * (newsample - newmean);
    if(newn <= 1)
      {
	fprintf(stderr, "REV: ERROR in incr compute var, newn <= 1, div by zero?!\n");
	exit(1);
      }
    float64_t newvar = M2n / (float64_t)(newn-1);
    prevM2n = M2n;
    //float64_t div = (newsample - prevmean);
    //float64_t newvar = (n-2)/(n-1)*prevvar + (1/n)*(div*div);
    return newvar;
  }


  
  std::vector<float64_t> dream_abc_state::compute_variance_for_all_dims_all_history()
  {
    //Keep a "partial sum" updated, and just do the change in N.
    //Also, keep a "deviation from mean" for each. Problem is, change in mean will cause change in the deviation...

    //http://math.stackexchange.com/questions/102978/incremental-computation-of-standard-deviation
    
    std::vector<float64_t> prev_var = state.get_last_row<float64_t>( X_variance_hist );
    std::vector<float64_t> prev_means = state.get_last_row<float64_t>( X_mean_hist );
    std::vector<float64_t> prev_M2ns = state.get_last_row<float64_t>( X_M2n_hist );

    int64_t nchains= get_param<int64_t>( N_chains_param );

    std::vector< std::vector<float64_t> > newsamples = state.get_last_n_rows<float64_t>( X_hist, nchains );

    int64_t n = state.get_num_rows( X_hist ) - newsamples.size();
    
    for(size_t c=0; c<newsamples.size(); ++c)
      {
	for(size_t d=0; d<prev_var.size(); ++d)
	  {
	    float64_t newsample = newsamples[c][d];
	    float64_t prevmean = prev_means[d];
	    ++n; //increment n, b/c we added a sample
	    prev_means[d] = incrementally_compute_mean( prev_means[d], newsample, n );
	    prev_var[d] = incrementally_compute_var( prevmean, prev_var[d], prev_means[d], newsample, n, prev_M2ns[d] );
	  }
      }

    //Write STDS, MEANS, M2NS. Note STD is sqrt of prev_stds.
    state.add_row_to_matrix( X_variance_hist, prev_var );
    state.add_row_to_matrix( X_mean_hist, prev_means );
    state.add_row_to_matrix( X_M2n_hist, prev_M2ns );
    
    return prev_var;
  }

  //REV: Whoops, I need to update DeltaCR for the USED INDICES!!!!
  void dream_abc_state::update_DeltaCR()
  {
    std::vector<std::vector<float64_t> > Xlast2gens = state.get_last_n_rows<float64_t>( X_hist, 2 * get_param<int64_t>( N_chains_param ) );

    std::vector<std::vector<float64_t> > lastgen =
      std::vector<std::vector<float64_t> >( Xlast2gens.begin(), Xlast2gens.begin()+get_param<int64_t>( N_chains_param ) );
    
    std::vector<std::vector<float64_t> > thisgen =
      std::vector<std::vector<float64_t> >( Xlast2gens.begin() + get_param<int64_t>( N_chains_param ), Xlast2gens.end() );

    std::vector<std::vector<int64_t> > usedCRidxs = state.get_last_n_rows<int64_t>( CR_used_hist, get_param<int64_t>( N_chains_param ) );

    std::vector<float64_t> DeltaCR = state.get_last_row<float64_t>( DeltaCR_hist );
    
    if(thisgen.size() != lastgen.size() )
      {
	fprintf(stderr, "REV: ERROR fuxxed up iterarors in updateDELTACR\n");
	exit(1);
      }
    
    std::vector<float64_t> vars = compute_variance_for_all_dims_all_history();

    //SQRT BECAUSE THEY ARE ACTUALLY VARIANCE...
    vector_sqrt<float64_t>( vars );
    
    size_t ndims = get_param<int64_t>( d_dims_param );
    //Update DeltaCR (norm squared jump distances...)
    for(size_t c=0; c<lastgen.size(); ++c)
      {
	size_t used = usedCRidxs[c][0];
	float64_t dist=0;
	for(size_t d=0; d<ndims; ++d)
	  {
	    if(vars[d] > 0 )
	      {
		double a=( lastgen[c][d] - thisgen[c][d] ) / ( vars[d] );
		dist += a*a;
	      }
	    
	  }
	DeltaCR[used] += dist;
	
      }
    
    //Update/write DeltaCR (add row to history)
    state.add_row_to_matrix( DeltaCR_hist, DeltaCR );
    
  }
  
  
  //REV: Huh, there's probably a more intelligent way to do this than manually check each one?
  std::vector<size_t> dream_abc_state::choose_moving_dims( const size_t& Didx,
					  std::default_random_engine& rand_gen ) 
  {
    std::uniform_real_distribution<float64_t> udist(0.0, 1.0);
    
    std::vector<size_t> mymovingdims;
    
    //Number to choose / nCR. I.e. lowest is 1.0/nCR.
    float64_t crval = (float64_t)(Didx+1) / (float64_t)get_param<int64_t>( nCR_param );
    //fprintf(stdout, "CRVAL is [%lf]\n", crval);
    size_t ndims=get_param<int64_t>( d_dims_param );
    for(size_t d=0; d <ndims; ++d)
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
    
    return mymovingdims;
  }
  
  void dream_abc_state::choose_moving_dims_and_npairs( std::vector<size_t>& mypairs,
				      std::vector<size_t>& moving_dims,
				      float64_t& gamma,
				      std::default_random_engine& rand_gen )
  {
    size_t ndims = get_param<int64_t>( d_dims_param );
    size_t Didx = choose_CR_index( rand_gen );
    size_t tauidx = draw_num_DE_pairs( rand_gen );
    gamma = compute_gamma_nonsnooker( tauidx+1, Didx+1, rand_gen );
    if( gamma == 1.0 )
      {
	//Didx = get_param<int64_t>(d_dims_param) - 1; //-1 bc its idx.
	//REV: OH NO, *probability* of selecting any given dim in d_dims_param is ONE, so we want nCR-1, because that way (nCR/(nCR-1+1)) == 1
	Didx = get_param<int64_t>( nCR_param ) - 1;
	tauidx = 0;
      }

    //Each pair gets the same pair of moving dims.
    moving_dims = choose_moving_dims( Didx, rand_gen );
    
    mypairs = draw_DE_pairs( tauidx+1, rand_gen );

    if(gamma==1 && ( (moving_dims.size() != ndims) || mypairs.size() != 2 ))
      {
	fprintf(stderr, "REV* error, even though gamma==1, moving dims is not full!\n");
	exit(1);
      }
    
    //Assume we're pushing back to the correct "new" chain at the end.
    state.add_row_to_matrix<int64_t>( CR_used_hist, std::vector<int64_t>(1, Didx ));
    
    //state.add_row_to_matrix<int64_t>( delta_used_hist, std::vector<int64_t>(1, tauidx ));
    
    //REV: Update used_CR, used_delta, etc.
    //Also compute STD, etc., and move them based on those?
    //Actually update pCR and pDELTA?
    //pDELTA is not updated (it's always equal for each num pairs..)
    //Every generation, we update DeltaCR.
    //Once every SKIP, we update pCR.
    
    return;
  }

  size_t dream_abc_state::choose_CR_index( std::default_random_engine& rand_gen )
  {
    std::vector<float64_t> pcr = state.get_last_row<float64_t>( pCR_hist );
    if( get_param<int64_t>(nCR_param) > 1 )
      {
	std::vector<size_t> chosen = multinomial_sample( pcr, 1, rand_gen ); //PCR is a problem...?
	/*std::vector<size_t>::iterator it = std::find(chosen.begin(), chosen.end(), 1);
	if( it == chosen.end() )
	  {
	    fprintf(stderr, "ERROR couldnt find chosen in multinomial!!!??!?!\n");
	    exit(1);
	  }
	*/
	size_t idx=0;
	for(size_t x=0; x<pcr.size(); ++x)
	  {
	    if( chosen[x] == 1 )
	      {
		idx = x;
	      }
	  }
	/*if( idx != *it )
	  {
	    fprintf(stderr, "ERROR, std FIND is not doing what the heck it should in chooseCRidx!!!\n");
	    exit(1);
	  }
	*/
	//return *it;
	return idx;
	//Find which one has 1 in it...
	//return chosen[0];
      }
    else
      {
	return 0;
      }
  }
  
  //REV: dprime/D is the number of moving dimensions, tau is the number of pairs being used...
  float64_t dream_abc_state::compute_gamma_nonsnooker(const size_t& tau,
				     const size_t& D,
				     std::default_random_engine& rand_gen )
  {

    std::uniform_real_distribution<float64_t> udist(0.0, 1.0);
    
    float64_t pgamma1 = get_param<float64_t>( p_jump_param );
    if( udist(rand_gen) < pgamma1 )
      {
	//fprintf(stdout, "PGAMMA IS 1!\n");
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
  size_t dream_abc_state::draw_num_DE_pairs( std::default_random_engine& rand_gen )
  {
    //Basically draws from PDELTA, sets it, etc.
    //std::vector<float64_t> pdelta = get_vector<float64_t>( pdelta_state );
    std::vector<float64_t> pdelta = get_vector_param<float64_t>( pdelta_param );
    std::vector<size_t> npairs = multinomial_sample( pdelta, 1, rand_gen );
    size_t idx=0;
    for(size_t x=0; x<npairs.size(); ++x)
      {
	if(npairs[x] == 1)
	  {
	    idx=x;
	  }
      }
    return idx;
  }
  
  std::vector<size_t>  dream_abc_state::draw_DE_pairs( const size_t& npairs,
					     std::default_random_engine& rand_gen )
  {
    return choose_k_indices_from_N_no_replace( get_param<int64_t>(N_chains_param), npairs*2, rand_gen );
  }

  
  bool dream_abc_state::compute_GR()
  {
    //fprintf(stdout, "Computing GR\n");
    int64_t timepoints = get_param<int64_t>(t_gen) / 2;
    if( timepoints < 2 )
      {
	return false;
      }
    //REV: Rofl, doesn't complain about the type...
    size_t ndims = get_param<int64_t>( d_dims_param );
    size_t nchains = get_param<int64_t>( N_chains_param );
    size_t nrows = state.get_num_rows( X_hist );
    //Reconstruct the chain, and take only 2nd half

    size_t hstart = nrows-(timepoints*nchains);
    
    //REV: This is a pain in the ass, because it has to be from the last
    //50%. So, it's not possible to incrementally do it I think.
    //However, I can at least incrementally compute STD as I go.
    
    std::vector<std::vector<float64_t> > X_half_hist = state.read_row_range<float64_t>( X_hist, hstart, hstart+nchains-1 ); //e.g. zero to 10, if inclusive.
    
    std::vector< std::vector< float64_t> > each_chain_and_dim_means( nchains, std::vector<float64_t>( ndims ) ); //will fill with first value bc n=1
    for(size_t c=0; c<nchains; ++c)
      {
	each_chain_and_dim_means[c] = X_half_hist[c];
      }
    std::vector< std::vector< float64_t> > each_chain_and_dim_vars( nchains, std::vector<float64_t>( ndims, 0 ) );
    std::vector< std::vector< float64_t> > chainM2n( nchains, std::vector<float64_t>( ndims, 0 ) );
    
    size_t n=1;
    
    for(size_t t=1; t<timepoints; ++t) //(nchains+c); t<(timepoints*nchains); t+=nchains)
      {
	++n;
	size_t tstart = t*nchains;

	
	
	X_half_hist = state.read_row_range<float64_t>( X_hist, hstart+tstart, hstart+tstart+nchains-1 );
	
	
	//for each chain:
	for(size_t c=0; c<nchains; ++c)
	  {
	    if(X_half_hist.size() <= c)
	      {
		fprintf(stderr, "ERROR X half hist size too small\n");
		exit(1);
	      }
	    
	    //for each dim
	    for(size_t d=0; d<ndims; ++d)
	      {
		float64_t sample = X_half_hist[c][d];
		float64_t newmean = incrementally_compute_mean( each_chain_and_dim_means[c][d], sample, n);
		float64_t newvar = incrementally_compute_var( each_chain_and_dim_means[c][d], each_chain_and_dim_vars[c][d], newmean, sample, n, chainM2n[c][d] );
		each_chain_and_dim_means[c][d] = newmean;
		each_chain_and_dim_vars[c][d] = newvar;
	      } //end for all dims
	  } //end for all chains
      } //end for all timepoints

    for(size_t c=0; c<nchains; ++c)
      {
	fprintf(stdout, "\n(GR) Chain [%ld] mean: ", c);
	print1dvec_row<float64_t>( each_chain_and_dim_means[c] );
	
	fprintf(stdout, "\n(GR) Chain [%ld] variance: ", c);
	print1dvec_row<float64_t>( each_chain_and_dim_vars[c] );
	
	fprintf(stdout, "\n");
      }
    std::vector<float64_t> variance_between_chain_means(ndims, 0);
    std::vector<float64_t> means(ndims, 0);
    
    for(size_t c=0; c<nchains; ++c)
      {
	//for each dim:
	for(size_t d=0; d<ndims; ++d)
	  {
	    means[d] += each_chain_and_dim_means[c][d];
	  }

      }
    vector_divide_constant<float64_t>( means, (float64_t)nchains );
    fprintf(stdout, "(GR) MEAN among all chains: ");
    print1dvec_row<float64_t>( means );
    fprintf(stdout, "\n");
    
    
    for(size_t c=0; c<nchains; ++c)
      {
	for(size_t d=0; d<ndims; ++d)
	  {
	    float64_t tmp = (each_chain_and_dim_means[c][d] - means[d]);
	    variance_between_chain_means[d] += (tmp*tmp);
	  }
      }

    //This is VARIANCE of each chains mean from the mean of all the chains.
    
    vector_divide_constant<float64_t>(variance_between_chain_means, (float64_t)(nchains-1));
    fprintf(stdout, "(GR) VARIANCE (std^2) among all chains: ");
    print1dvec_row<float64_t>( variance_between_chain_means );
    fprintf(stdout, "\n");
    
                              
    //Wait, what the heck? Multiply by #timepoints??!?! (REV: This is because later we do N/(N-1) or some shit?
    vector_multiply_constant<float64_t>(variance_between_chain_means, (float64_t)timepoints);
    


    //What is the variance between chains
    std::vector<float64_t> mean_variance_all_chain_dim(ndims, 0);
    //4) Compute mean of within-sequence variances. Mean of each dim from step 3. Call W.
    for(size_t c=0; c<nchains; ++c)
      {
	for(size_t d=0; d<ndims; ++d)
	  {
	    mean_variance_all_chain_dim[d] += each_chain_and_dim_vars[c][d];
	  }
      }
    
    //This is the mean of the *variance* of each dimension. In other words, distance of each chains's variance, from the mean variance of all the chains
    //This represents MEAN of VARIANCE
    vector_divide_constant<float64_t>( mean_variance_all_chain_dim, (float64_t)nchains );

    fprintf(stdout, "GR: Mean of the variance among all chains:   ");
    print1dvec_row<float64_t>( mean_variance_all_chain_dim );
    
    std::vector<float64_t> Rstat(ndims, 0);

    //REV: SHould be:
    //R = SQRT(VAR(THETA) / W)
    //VAR(THETA)=(1-(1/T))*W + (1/T)*B
    //B=(T/(chains-1))* SUM(j->chains)(chainmean(j)-meanallchains) //variance of CHAIN MEANS (times T)
    //meanallchains = (1/chains)*SUM(j->chains)(chainmean(j))    //mean among all chain means
    //chainmean(j) = (1/T)*SUM(i->T)chain(ij)     //mean of chain j
    //chain(ij) is timepoint i of chain j      //value of chain j at timepoint i
    //s2(j) = 1/(T-1)*SUM(i->T)(chain(ij) - chainmean(j) )    //variance of chain j
    //W = 1/chains * SUM(j->chains) s2(j) //mean of variances.
    
    
    //T=#timepoints
    //C=#chains
    //I Have computed:
    //REV: Note my variance_between_chain_means is multiplied by T.
    // (T-1)/T + ((C+1)/(C*T))*( var_btw_chain_means / mean_var_all_chain_dim )

    //(T-1)/T + (C/C*T + 1/C*T)* ( var_btw_chain_means/mean_var_all_chain_dim )
    
    
    //V / W
    //((1-(1/T))*W + (1/T)*B)   / W
    //((1-(1/T))*( mean_var_all_chain_dim ) + ( (1/T)* (var_btw_chain_means) )
    //Note, all div by W, so:
    //(1-(1/T)) + (((1/T) * (var_btw_chain_means)) / mean_var_all_chain_dim)
    

    //REV: My first term is equilvanet( 1-(1/x) == (x-1)/x )

    //Now, is: (C/C*T + 1/C*T) ==
    //(1/T) ? ( C+1  / C*T ) (C/C*T + 1/(C*T) = 1/T + 1/(C*T). So I may have been off by a factor of 1/(C*T), which is negligible...oops.
    //(C/(C*T) + 1/(C*T)) == (C+1)/(C*T).

        
    
    bool wouldconverge = true;
    for(size_t d=0; d<ndims; ++d)
      {
	if(mean_variance_all_chain_dim[d] > 0)
	  {
	    /*Rstat[d] = (float64_t)(timepoints-1)/(float64_t)timepoints +
	      ( (float64_t)(nchains+1)/(float64_t)(nchains*timepoints) ) *
	      ( (float64_t)variance_between_chain_means[d] / (float64_t)mean_variance_all_chain_dim[d] );*/
	    Rstat[d] = (float64_t)(timepoints-1)/(float64_t)timepoints +
	      ( (float64_t)1.0/(float64_t)(timepoints) ) *
	      ( (float64_t)variance_between_chain_means[d] / (float64_t)mean_variance_all_chain_dim[d] );
	    
	    Rstat[d] = sqrt( Rstat[d] );
	  }
	else
	  {
	    Rstat[d] = 6666.0;
	  }
	
	if( Rstat[d] >= get_param<float64_t>( GR_thresh_param ) )
	  {
	    //At least one dim has R value > threshold
	    wouldconverge = false;
	  }
      }

    state.add_row_to_matrix<float64_t>( GR_hist, Rstat );
    fprintf(stdout, "Completed GR computation:   ");
    print1dvec_row<float64_t>( Rstat );
    return wouldconverge;
  } //end compute_GR


  
  void dream_abc_state::update_pCR()
  {
    //fprintf(stdout, "UPDATING PCR!!!\n");
    std::vector<float64_t> DeltaCR = state.get_last_row<float64_t>( DeltaCR_hist );
    std::vector<int64_t> CRcnts = state.get_last_row<int64_t>( CR_cnts_hist );

    std::vector<float64_t> pCR = state.get_last_row<float64_t>( pCR_hist );
    
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
	for(size_t cridx=0; cridx<ncr; ++cridx)
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

    fprintf(stdout, "Gen [%ld] pCR: ", get_param<int64_t>( t_gen ));
    print1dvec_row( pCR );
    //fprintf(stdout, "FINISHED Computed PCR!\n");
  }

  
  
  
  void dream_abc_state::generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg )
  {

    START_GEN();
    
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


  //We don't compute DeltaCR etc., but we need to populate the history of the state variables for incremental computation
  //of DeltaCR
  void dream_abc_state::init_population_mean_var()
  {
    //init to single value.
    state.add_to_matrix<float64_t>( X_mean_hist, state.get_last_n_rows<float64_t>( X_hist, get_param<int64_t>(N_chains_param) ) );

    //Init to zeroes
    state.add_to_matrix<float64_t>( X_variance_hist, std::vector<std::vector<float64_t>>( get_param<int64_t>(N_chains_param), std::vector<float64_t>( get_param<int64_t>(d_dims_param), 0.0 ) ) );

    //init to zeroes
    state.add_to_matrix<float64_t>( X_M2n_hist, std::vector<std::vector<float64_t>>( get_param<int64_t>(N_chains_param), std::vector<float64_t>( get_param<int64_t>(d_dims_param), 0.0 ) ) );
  }

  std::vector<float64_t> dream_abc_state::compute_stat_divergence( varlist<std::string>& results, std::vector<float64_t>& resultvals )
  {
    std::vector<float64_t> targ = get_vector_param<float64_t>( Y_param );
    const std::vector<std::string> names = state.get_varnames( Y_param );
    
    resultvals.resize( targ.size() );
    
    std::vector<float64_t> ret( targ.size() );
    
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
  //Absolute value of distance...
  std::vector<float64_t> dream_abc_state::compute_stat_abs( const std::vector<float64_t>& divergence )
  {
    std::vector<float64_t> ret( divergence );
    vector_sq<float64_t>( ret );
    vector_sqrt<float64_t>( ret );
    return ret;
  }

  std::vector<float64_t> dream_abc_state::compute_epsilon_divergence( const std::vector<float64_t>& abs_divergence )
  {
    //std::vector<float64_t> ret( abs_divergence );
    //vector_subtract_from_constant<float64_t>( get_param<float64_t>( epsilon_param ), ret );
    return
      ( vector_subtract<float64_t>( get_vector_param<float64_t>( epsilon_param ), abs_divergence ) );
  }

  float64_t dream_abc_state::compute_rho( const std::vector<float64_t>& epsilon_divergence )
  {
    //Same as MAX of phi-RHO
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
  
  //std::vector<bool> compute_acceptance( )
  void dream_abc_state::compute_acceptance()
  {
    const int64_t nchains = get_param<int64_t>(N_chains_param);
    std::vector< std::vector< float64_t> > Hfits = state.get_last_n_rows<float64_t>( piH_hist, nchains );
    std::vector< std::vector< float64_t> > Xfits = state.get_last_n_rows<float64_t>( piX_hist, nchains );

    fprintf(stdout, "OLDFIT: ");
    for(size_t c=0; c<Xfits.size(); ++c)
      {
	fprintf(stdout, "%5.3lf ", Xfits[c][0] );
      }
    fprintf(stdout, "\nNEWFIT: ");
    for(size_t c=0; c<Hfits.size(); ++c)
      {
	fprintf(stdout, "%5.3lf ", Hfits[c][0] );
      }
    fprintf(stdout, "\n\n");
    //fprintf(stdout, "Computing acceptance: got Hfits and Xfits\n");
    
    std::vector<int64_t> accept( nchains, 0 );
    for( size_t c=0; c<Hfits.size(); ++c )
      {
	//fprintf(stdout, "Computing for ACCEPT chain #[%ld]\n", c);
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
	//fprintf(stdout, "Added accept row to matrix #[%ld]\n", c);
      }
    fprintf(stdout, "ACCEPTS: ");
    print1dvec_row<int64_t>( accept );
    //return accept;
  }

  void dream_abc_state::move_chains()
  {
    //Uses accept_hist to move X to H.
    int64_t nchains =    get_param<int64_t>(N_chains_param);
    std::vector<std::vector<float64_t> > Xcurr = state.get_last_n_rows<float64_t>( X_hist, nchains );
    std::vector<std::vector<float64_t> > Hcurr = state.get_last_n_rows<float64_t>( H_hist, nchains );
    std::vector<std::vector<int64_t> > accepted = state.get_last_n_rows<int64_t>( accept_hist, nchains );
    
    //Get fitnesses too and use those...
    std::vector<std::vector<float64_t> > Xfit = state.get_last_n_rows<float64_t>( piX_hist, nchains );
    std::vector<std::vector<float64_t> > Hfit = state.get_last_n_rows<float64_t>( piH_hist, nchains );
    
    for( size_t c=0; c<Xcurr.size(); ++c )
      {
	if(accepted[c][0] == 1)
	  {
	    state.add_row_to_matrix<float64_t>( X_hist, Hcurr[c] );
	    state.add_row_to_matrix<float64_t>( piX_hist, Hfit[c] );
	  }
	else
	  {
	    state.add_row_to_matrix<float64_t>( X_hist, Xcurr[c] );
	    state.add_row_to_matrix<float64_t>( piX_hist, Xfit[c] );
	  }
      }
  }
  

  void dream_abc_state::compute_generation_fitnesses( const std::vector<std::vector<float64_t>>& vals, filesender& fs, parampoint_generator& pg  )
  {
    const std::vector<std::string> names = state.get_varnames( dim_mins_param );
    
    std::vector<varlist<std::string> > vls( vals.size() );
    
    //Make varlist from names and doubles...
    for(size_t x=0; x<vals.size(); ++x)
      {
	//fprintf(stdout, "TRYING TO COMPUTE:\n");
	for(size_t y=0; y<vals[x].size(); ++y)
	  {
	    // fprintf(stdout, "[%s] [%lf]\n", names[y].c_str(), vals[x][y]);
	  }
	varlist<std::string> vl;
	vl.make_varlist<float64_t>(names, vals[x] );
	vls[x] = vl;
      }
    
    
    //Compute the fitnesses of them
    fs.comp_pp_list( pg, vls, sg );


    //fprintf(stdout, "Finished computing fitnesses! Fitness results is: [%ld]\n", pg.parampoint_vars.size() );
    //TODO REV: get results from RESULTS, specifically FITNESS? Just get from RESULTS (should be a varlist heh)
    //We know same order as varlists we gave, so OK.
        
    //std::vector<varlist<std::string> > results = pg.get_last_N_results( get_param<int64_t>(N_chains_param) );
    std::vector<varlist<std::string> > results = pg.get_last_N_results( vals.size() );
    
    //fprintf(stdout, "Got last N results\n");
    
    pg.cleanup_parampoints_upto( vals.size() );

    //fprintf(stdout, "Cleaned up last N results\n");
    
    //Add these guys to H.
    state.add_to_matrix<float64_t>( H_hist, state.get_varnames( H_hist ), vals );
    
    //fprintf(stdout, "Added to matrix...\n");
    std::vector<float64_t> fitnesses( results.size(), -66666.22222 );
    
    int64_t tgen = get_param<int64_t>(t_gen);
    //TODO: Append results to piX and piH
    for(size_t x=0; x<results.size(); ++x)
      {
	//fprintf(stdout, "Comping result [%ld]...\n", x);
	std::vector<float64_t> statdiv;
	std::vector<float64_t> ed = compute_epsilon_divergence( compute_stat_abs( compute_stat_divergence( results[x], statdiv) ) );
	//fprintf(stdout, "Computed epsilon div [%ld]\n", x);
	//REV: statdiv contains RAW results per-name?
	float64_t fit = compute_rho( ed );
	//fprintf(stdout, "GEN [%ld]: Computed rho chain [%ld] (%lf)\n", tgen, x, fit);
	fitnesses[x] = fit;
	
	//The fitnesses are organized as column vectors...so I need to add "rows" one at a time lol...
	state.add_row_to_matrix<float64_t>( piH_hist, std::vector<float64_t>(1, fit) );
	state.add_row_to_matrix<float64_t>( model_observ_diverg_hist, statdiv );
      }
    fprintf(stdout, "FITS gen [%ld]:  ", tgen);
    print1dvec_row<float64_t>( fitnesses );

    //Fitness hist is only added after we check accept/not accept...
  } //end comp gen fitnesses


  void dream_abc_state::init_random()
  {
    std::random_device rd;
    rg.seed(rd());
    
    sg.seed( rd() );
  }

  void dream_abc_state::init_random(const long& seed)
  {
    rg.seed( seed );
    
    sg.seed( seed );
  }

  void dream_abc_state::initialize( )
  {
    init_random();
  }

  void dream_abc_state::initialize(const long& seed )
  {
    init_random(seed);
  }

  //Are these "loaded" at load time? NO!!! They aren't. We don't want these dirty state variables around...
  //Well, statefilename might be fine, as varfilename, etc.
  


  //REV: UGLY UGLY UGLY WIll be MASKED By derived..(for now)
  void dream_abc_state::create_with_config( dream_abc_state::abcconfig& c )
    {
      //First, check if we need to load from statefile.
      if(c.restart)
	{
	  bool isbackup=false;
	  load_state( c._statefilename, isbackup ); //Don't actually need to load any state variables as everything is stored in HDF5.
	  
	  //State is now loaded, we can assume that no other variables
	  //will be changed except for maxgens will be set.
	  fprintf(stdout, "(RE)-initializing MAXGENS from original [%ld] to new [%ld]\n", get_param<int64_t>(T_max_gens_param), c._maxgens);
	  
	  set_param<int64_t>( T_max_gens_param, c._maxgens );
	}
      else
	{
	  parse_dreamabcfile( c._varfilename, c._varnames, c._mins, c._maxes );
	  parse_obsdatafile( c._obsfilename, c._observation_varnames, c._observation_stats );
	  parse_epsilondatafile( c._epsilonsfilename, c._observation_varnames, c._observation_epsilons );

	  if( c.configexists )
	    {
	      c.load_abc_params( c._CONFIGFILE );
	    }
	  
	  new_state( c._statefilename,
		     c._varnames,
		     c._mins,
		     c._maxes,
		     c._observation_varnames,
		     c._observation_stats,
		     c._observation_epsilons,
		     c._maxgens,
		     c._numchains,
		     c._ndelta,
		     c._bnoise,
		     c._bstar,
		     c._rthresh,
		     c._GRskip,
		     c._nCR,
		     c._pCRskip,
		     c._pjump,
		     c._backupskip
		     );
	}
    }

  
  //REV: UGLY UGLY will be MASKED by derived!!! (for now)
  //If we actually modify things "in-line" that might be better.
  //But user creation function might have a different view of things.
  //But, other than this, everything is default I assume?
  dream_abc_state::abcconfig dream_abc_state::parseopts( optlist& opts )
  {
    abcconfig conf;

    
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    //"REQUIRED" parameters (to load or start new)

   auto c = opts.get_opt_args( "RESTART" );
    if( c.size() == 0 )
      {
      	fprintf(stdout, "DREAM ABC: User did not specify a RESTART option. A new HDF5 COLLECTION will be created for this search at [%s], whether or not it exists!!!\n", conf._statefilename.c_str());
	conf.restart = false;
      }
    else
      {
	if(c[0].size() > 0 )
	  {
	    conf._statefilename = opts.get_opt( "RESTART" ).get_arg( 0 ); 
	  }
	else
	  {
	    fprintf(stderr, "ERROR in RESTART: expects at least one argument: Name of state file to restart from...\n");
	    exit(1);
	  }
	fprintf(stdout, "DREAM ABC: User specified **RESTART** option. The previously created HDF5 Collection [%s] will be used for this search!!! (note: An additional argument can be added to this option, an integer value MAXGENS that specifies net MAXGENS)\n", conf._statefilename.c_str());
	conf.restart = true;
	
	if(c[0].size() > 1)
	  {
	    conf._maxgens = opts.get_opt( "RESTART" ).argn_as_int64( 0 );
	  }
      }
    
    
    //Parse to load varnames, mins, maxes, etc.
    c = opts.get_opt_args( "VARIABLES" );

    if( c.size() == 0 )
      {
	if( !conf.restart )
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, did *not* specify a -VARIABLES for running, please specify and run again\n");
	    exit(1);
	  }
      }
    else
      {
	if(c[0].size() == 0 || c[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, -VARIABLES option had [%ld] arguments, expects only 1 (File name containing VARIABLES data)\n", c[0].size() );
	    exit(1);
	  }
	else
	  {
	    conf._varfilename = c[0][0];
	  }
	fprintf(stdout, "DREAM ABC: Using specified VARIABLES [%s] for running search\n", conf._varfilename.c_str() );
      }
    

    c = opts.get_opt_args( "OBSERVATIONS" );
    if( c.size() == 0 )
      {
	if( !conf.restart )
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, did *not* specify a -OBSERVATIONS for running, please specify and run again\n");
	    exit(1);
	  }
      }
    else
      {
	if(c[0].size() == 0 || c[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, -OBSERVATIONS option had [%ld] arguments, expects only 1 (File name containing OBSERVATIONS data)\n", c[0].size() );
	    exit(1);
	  }
	else
	  {
	    conf._obsfilename = c[0][0];
	  }
	fprintf(stdout, "DREAM ABC: Using specified OBSERVATIONS [%s] for running search\n", conf._obsfilename.c_str() );
      }

    c = opts.get_opt_args( "EPSILONS" );
    if(  c.size() == 0 )
      {
	if(!conf.restart )
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, did *not* specify a -EPSILONS for running, please specify and run again\n");
	    exit(1);
	  }
      }
    else
      {
	if(c[0].size() == 0 || c[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, -EPSILONS option had [%ld] arguments, expects only 1 (File name containing EPSILONS data)\n", c[0].size() );
	    exit(1);
	  }
	else
	  {
	    conf._epsilonsfilename = c[0][0];
	  }
	fprintf(stdout, "DREAM ABC: Using specified EPSILONS [%s] for running search\n", conf._epsilonsfilename.c_str() );
      }

    

    c = opts.get_opt_args( "STATEFILE" );
    if( c.size() == 0 )
      {
	if(!conf.restart)
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, did *not* specify a -STATEFILE for running, please specify and run again\n");
	    exit(1);
	  }
      }
    else
      {
	if(c[0].size() == 0 || c[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR DREAM ABC: Parseopts, -STATEFILE option had [%ld] arguments, expects only 1 (File name that will be STATEFILE)\n", c[0].size() );
	    exit(1);
	  }
	else
	  {
	    conf._statefilename = c[0][0];
	  }
	fprintf(stdout, "DREAM ABC: Using specified STATEFILE [%s] for running search. If it exists, you must specify --RESTART to restart it (you should also increase e.g. max generations). Otherwise, it will be rewritten/overwritten.\n", conf._statefilename.c_str() );
      }

    
    

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    //"OPTIONAL" parameters (config file parameters)
    
    c = opts.get_opt_args( "ABCPARAMS" );
    if( c.size() > 0 )
      {
	if(c[0].size() != 1)
	  {
	    fprintf(stderr, "DREAM ABC PARSING OPTIONS: ERROR: ABCPARAMS had [%ld] arguments but was expecting just 1 (file containing parameters for ABC)\n", c[0].size());
	    exit(1);
	  }
	else
	  {
	    conf._CONFIGFILE=c[0][0];
	    conf.configexists=true;
	    fprintf(stdout, "DREAM ABC: setting ABCCONFIG file to [%s], from which I will load ABC configuration. Command line parameters will take precedence over CONFIG file parameters (which take precdence over defaults)\n", conf._CONFIGFILE.c_str());
	  }
      }
    else
      {
	fprintf(stdout, "DREAM ABC: NOTE: User did not specify an ABCPARAMS file, so only command line parameters and defaults will be used (ignore this if you are restarting a run, in which case it is stored in the state file)\n");
	conf.configexists=false;
      }
    
    c = opts.get_opt_args( "MAXGENS" );
    if( c.size() > 0 )
      {
	if(c[0].size() != 1)
	  {
	    fprintf(stderr, "DREAM ABC PARSING OPTIONS: ERROR: MAXGENS had [%ld] arguments but was expecting just 1 (number of gens)\n", c[0].size());
	    exit(1);
	  }
	else
	  {
	    conf._maxgens = opts.get_opt( "MAXGENS" ).argn_as_int64( 0 );
	    fprintf(stdout, "DREAM ABC: setting MAXGENS to [%ld], which may be greater than current (loaded) maxgens. Note, command line parameters will take precedence over CONFIG file parameters, which will take precedence over currently existing (saved) parameters in a loaded STATE file\n", conf._maxgens);
	  }
      }


    c = opts.get_opt_args( "NCHAINS" );
    if( c.size() > 0 )
      {
	if(c[0].size() != 1)
	  {
	    fprintf(stderr, "DREAM ABC PARSING OPTIONS: ERROR: NCHAINS had [%ld] arguments but was expecting just 1 (number of chains)\n", c[0].size());
	    exit(1);
	  }
	else
	  {
	    conf._numchains = opts.get_opt( "NCHAINS" ).argn_as_int64( 0 );
	    fprintf(stdout, "DREAM ABC: setting NCHAINS to [%ld]. Note, command line parameters will take precedence over ABCPARAMS file parameters, which take precedence over defaults\n", conf._numchains);
	  }
      }

    return conf;
    
  } //end parseopts

  
  void dream_abc_state::parse_dreamabcfile( const std::string& fname, std::vector<std::string>& _varnames, std::vector<float64_t>& _mins, std::vector<float64_t>& _maxes )
  {
    bool hascolnames = true;
    data_table dtable( fname, hascolnames );

    fprintf(stdout, "DREAMABC: PARSE ABCFILE: Trying to get VARNAMEs\n");
    _varnames = dtable.get_col( "NAME" );
    fprintf(stdout, "Got varnames. Getting MINS\n");
    _mins = data_table::to_float64( dtable.get_col( "MIN" ) );
    fprintf(stdout, "Got mins, getting MAXES\n");
    _maxes = data_table::to_float64( dtable.get_col( "MAX" ) );
    fprintf(stdout, "Got MAX. Finished parse of dreamabcfile\n");

    return;
  }

  void dream_abc_state::parse_obsdatafile( const std::string& observfname, std::vector<std::string>& _observation_varnames, std::vector<float64_t>& _observation_stats )
  {
    bool hascolnames = true;
    
    
    fprintf(stdout, "Getting observ data from [%s]\n", observfname.c_str() );
    data_table obsvdtable( observfname, hascolnames );
    
    fprintf(stdout, "DREAMABC: PARSE OBSERVERED DATA FILE: Trying to get VARNAMEs\n");
    
    _observation_varnames = obsvdtable.get_col( "NAME" );

    fprintf(stdout, "DREAMABBC: Got observation file NAMES, now getting STATS\n");
    _observation_stats = data_table::to_float64( obsvdtable.get_col( "VAL" ) ); //REV: this will just be ERROR and 0 for me... heh.

    fprintf(stdout, "DREAMABC: Got OBSERVATION STATS, now done with parse obsdatafile\n");
    
    return;
  }

  void dream_abc_state::parse_epsilondatafile( const std::string& epsilfname, const std::vector<std::string>& _varnames, std::vector<float64_t>& _epsilons )
  {
    bool hascolnames = true;
    
    
    fprintf(stdout, "Getting EPSILONS data from [%s]\n", epsilfname.c_str() );
    data_table epsiltable( epsilfname, hascolnames );
    
    fprintf(stdout, "DREAMABC: PARSE EPSILONS DATA FILE: Trying to get VARNAMEs\n");

    
    auto myvarnames = epsiltable.get_col( "NAME" );
    if(myvarnames.size() != _varnames.size() )
      {
	fprintf(stderr, "PARSE EPSILON DATA FILE: varname of observations and received myvarnames not same size [%ld] in epsilons vs [%ld] in obs\n", myvarnames.size(), _varnames.size() );
	exit(1);
      }
    for(size_t x=0; x<myvarnames.size(); ++x)
      {
	if( myvarnames[x].compare( _varnames[x] ) != 0 )
	  {
	    fprintf(stderr, "ERROR (?): myvarnames vs observation varnames in epsilon parse: order is not same or something is messed up!?! Varname idx [%ld] is different (epsilon [%s] vs obs [%s]). Will make this more robust later, for now, line them up!\n", x, myvarnames[x].c_str(), _varnames[x].c_str() );
	    exit(1);
	  }
      }

    fprintf(stdout, "DREAMABBC: Got epsilons file NAMES, now getting EPSILONS\n");
    _epsilons = data_table::to_float64( epsiltable.get_col( "EPSILON" ) ); //REV: this will just be ERROR and 0 for me... heh.

    fprintf(stdout, "DREAMABC: Got EPSILON, now done with parse epsilon datafile\n");
    
    return;
  }
  
  //CTOR
  dream_abc_state::dream_abc_state( optlist& opts )
  {
    initialize();

    abcconfig c = parseopts( opts );
    
    create_with_config(c);
    
    //load, etc.
  }
  
  //CTOR
  dream_abc_state::dream_abc_state()
  {
    initialize();
  }

  dream_abc_state::dream_abc_state( const long& seed) //Seed?
  {
    initialize( seed );
  }


  //Sample from X_hist etc.

  void dream_abc_state::enumerate_to_file( const std::string& matname, const std::string& fname, const size_t& thinrate, const size_t& startpoint )
  {
    fprintf(stdout, "Enumerating matrix [%s] to file: [%s], thinrate [%ld], startpoint [%ld]\n", matname.c_str(), fname.c_str(), thinrate, startpoint );
    state.enumerate_matrix_to_file( matname, fname, thinrate, startpoint );
  }
  
  //Note, do we only want to do it after convergence?
  void dream_abc_state::enumerate_X_GR_fitness( const std::string& dir, const size_t& genskiprate, const size_t& startgen )
  {
    make_directory( dir );
    std::string fnamebase = dir + "/";
    
    
    //REV: This separates for all chains? Ahhhhhhh Need to do that ;)
    size_t nchains = get_param<int64_t>( N_chains_param );
    
    //compute starting point from starting gen;
    size_t startpoint = nchains*startgen;

    //REV: Would really like to add a column at beginning telling generation... since it's row-first though, that's nasty...
    for(size_t c=0; c<nchains; ++c)
      {

	
	size_t startpointc = c+startpoint;

	std::string Xfname = fnamebase + "Xhist_" + std::to_string(c);
	std::string piXfname = fnamebase + "piXhist_" + std::to_string(c);
	fprintf(stdout, "Printing chain [%ld] Xhist to [%s] and piXhist to [%s]\n", c, Xfname.c_str(), piXfname.c_str());
	
	enumerate_to_file( X_hist, Xfname, nchains*genskiprate, startpointc);
	enumerate_to_file( piX_hist, piXfname, nchains*genskiprate, startpointc);
      }

    std::string GRfname = fnamebase + "GRhist";
    enumerate_to_file( GR_hist, GRfname , 1, 0); //could compute from GR_skip, but meh.

    return;
  }
    




void search_dream_abc( optlist& opts,
		       parampoint_generator& pg,
		       filesender& fs
		       )
{
  dream_abc_state state( opts );
  
    
  state.run( fs, pg );

  return;
}




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
		       )
{
  dream_abc_state state;

  float64_t oldepsil=0.05;
  std::vector<float64_t> epsils( observed_data.size(), oldepsil );
  //Should I load or not?
  state.new_state( statefilename,
		   varnames,
		   mins,
		   maxes,
		   observed_varnames,
		   observed_data,
		   epsils
		   );
  
  state.run( fs, pg );

  return;
}

