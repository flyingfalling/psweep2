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

typedef double float64_t;
typedef long int int64_t;

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
  
  D( epsilon_param );
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
			 float64_t epsil=2.0,
			 int64_t maxgens=2e4,
			 int64_t numchains=100,
			 int64_t ndelta=3,
			 float64_t bnoise=0.05,
			 float64_t bstar=1e-6,
			 float64_t rthresh=1.2,
			 int64_t GRskip=50,
			 int64_t nCR=3,
			 int64_t pCRskip=10,
			 float64_t pjump=0.05,
			 float64_t backupskip=10
			 )
  {
    state.new_collection( statefilename );
    
    //Observations of data from real world, that we will fit.
    state.add_float64_vector( Y_param, observation_varnames, observation_stats );
    
    state.add_float64_matrix( model_observ_diverg_hist, observation_varnames );

    state.add_float64_parameter( epsilon_param, epsil );
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

  
  
  void load_state( const std::string& file )
  {
    state.load_collection( file );

    //Check HDF5 file is "sane", i.e. that all matrices have appropriate length given t_current_gen etc. If not, we will try the equivalent BU file.
    bool sane = issane( );
    
    if(sane == false && file.c_str()[0] != '_' && file.c_str()[1] != '_' )
      {
	std::string bufname = "__" + file;
	state.load_collection( bufname );
      }
    else
      {
	fprintf(stderr, "ERROR: DREAM ABC Even backup file of h5 file [%s] is not sane! Exiting\n", file.c_str() );
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
    return state.get_numeric_parameter<T>( s );
  }

  template <typename T>
  void set_param( const std::string& s, const T& val)
  {
    state.set_numeric_parameter<T>( s, val );
  }
  
  template <typename T>
  std::vector<T> get_vector_param( const std::string& s )
  {
    return state.get_last_row<T>( s );
  }

  //Just use raw add to matrix...do everything first raw, then wrap it?

  void START_GEN()
  {
    set_param<int64_t>( PRE_GEN_ITER, get_param<int64_t>(PRE_GEN_ITER)+1 );
  }

  void END_GEN()
  {
    set_param<int64_t>( t_gen, get_param<int64_t>(t_gen)+1 );
    set_param<int64_t>( POST_GEN_ITER, get_param<int64_t>(POST_GEN_ITER)+1 );
  }
  
  void run( filesender& fs, parampoint_generator& pg )
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

  void run_generation(filesender& fs, parampoint_generator& pg)
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
  }

  virtual void cleanup_gen()
  {
    int64_t tgen = get_param<int64_t>(t_gen);
    int64_t crskip = get_param<int64_t>(pCR_skip_param );
    int64_t grskip = get_param<int64_t>(GR_skip_param );
    int64_t backupskip = get_param<int64_t>(backup_regularity_param);
    
    if( (tgen+1) % crskip == 0 )
      {
	update_pCR();
      }
    
    if( (tgen+1) % grskip == 0)
      {
	bool wouldconverge = compute_GR();
      }

    if( (tgen+1) % backupskip == 0 )
      {
	state.backup();
      }
    
  }

  //REV: Better to make a "pure virtual" and not derive from dream_abc...but some shared one?
  virtual std::vector<std::vector<float64_t> > get_mypairs_vectors( const std::vector<size_t>& pairidxs )
  {
    //For normal dream ABC, get current gen
    std::vector<std::vector<float64_t>> Xcurr = get_current_gen();
    std::vector<std::vector<float64_t> > mypairs = indices_to_vector_slices< std::vector<float64_t> >(Xcurr, pairidxs );
    return mypairs;
    
  }
  
  std::vector<float64_t> make_single_proposal( const size_t& mychainidx,
					       const std::vector<std::vector<float64_t>>& Xcurr,
					       std::default_random_engine& rand_gen )
  {

    std::vector<float64_t> parent = Xcurr[mychainidx];
    std::vector<float64_t> proposal = parent;
    
    
    std::vector<size_t> pairidxs;
    std::vector<size_t> movingdims;
    float64_t gamma;

    choose_moving_dims_and_npairs( pairidxs, movingdims, gamma, rand_gen );

    fprintf(stdout, "Pair idxs: ");
    print1dvec_row<size_t>( pairidxs );
    
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

  std::vector<float64_t> edge_handling( const std::vector<float64_t>& proposal,
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

  
  std::vector<std::vector<float64_t> > get_current_gen()
  {
    size_t nchains = get_param<int64_t>(N_chains_param);
    return ( state.get_last_n_rows<float64_t>( X_hist, nchains ) );
  }
  
  std::vector<std::vector< float64_t> > make_proposals(std::default_random_engine& rand_gen)
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

  void update_CRcnts()
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
  
  float64_t incrementally_compute_mean( const float64_t& prevmean, const float64_t& newsample, const int64_t& newn )
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
  float64_t incrementally_compute_var( const float64_t& prevmean,
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
  
  std::vector<float64_t> compute_variance_for_all_dims_all_history()
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
  void update_DeltaCR()
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
  std::vector<size_t> choose_moving_dims( const size_t& Didx,
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
  
  void choose_moving_dims_and_npairs( std::vector<size_t>& mypairs,
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

  size_t choose_CR_index( std::default_random_engine& rand_gen )
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
  float64_t compute_gamma_nonsnooker(const size_t& tau,
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
  size_t draw_num_DE_pairs( std::default_random_engine& rand_gen )
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
  
  virtual std::vector<size_t> draw_DE_pairs( const size_t& npairs,
					     std::default_random_engine& rand_gen )
  {
    return choose_k_indices_from_N_no_replace( get_param<int64_t>(N_chains_param), npairs*2, rand_gen );
  }

  
  bool compute_GR()
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
  
  void update_pCR()
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

  
  
  
  virtual void generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg )
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
  void init_population_mean_var()
  {
    //init to single value.
    state.add_to_matrix<float64_t>( X_mean_hist, state.get_last_n_rows<float64_t>( X_hist, get_param<int64_t>(N_chains_param) ) );

    //Init to zeroes
    state.add_to_matrix<float64_t>( X_variance_hist, std::vector<std::vector<float64_t>>( get_param<int64_t>(N_chains_param), std::vector<float64_t>( get_param<int64_t>(d_dims_param), 0.0 ) ) );

    //init to zeroes
    state.add_to_matrix<float64_t>( X_M2n_hist, std::vector<std::vector<float64_t>>( get_param<int64_t>(N_chains_param), std::vector<float64_t>( get_param<int64_t>(d_dims_param), 0.0 ) ) );
  }

  std::vector<float64_t> compute_stat_divergence( varlist<std::string>& results, std::vector<float64_t>& resultvals )
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
  std::vector<float64_t> compute_stat_abs( const std::vector<float64_t>& divergence )
  {
    std::vector<float64_t> ret( divergence );
    vector_sq<float64_t>( ret );
    vector_sqrt<float64_t>( ret );
    return ret;
  }

  std::vector<float64_t> compute_epsilon_divergence( const std::vector<float64_t>& abs_divergence )
  {
    std::vector<float64_t> ret( abs_divergence );
    vector_subtract_from_constant<float64_t>( get_param<float64_t>( epsilon_param ), ret );
    return ret;
  }

  float64_t compute_rho( const std::vector<float64_t>& epsilon_divergence )
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
  void compute_acceptance()
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

  void move_chains()
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
  

  void compute_generation_fitnesses( const std::vector<std::vector<float64_t>>& vals, filesender& fs, parampoint_generator& pg  )
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


  void init_random()
  {
    std::random_device rd;
    rg.seed(rd());
    
    sg.seed( rd() );
  }

  void init_random(const long& seed)
  {
    rg.seed( seed );
    
    sg.seed( seed );
  }

  void initialize( )
  {
    init_random();
  }

  void initialize(const long& seed )
  {
    init_random(seed);
  }
  
  
  //CTOR
  dream_abc_state()
  {
    initialize();
  }

  dream_abc_state( const long& seed) //Seed?
  {
    initialize( seed );
  }
  
};


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




