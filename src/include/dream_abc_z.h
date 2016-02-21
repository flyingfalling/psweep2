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
  void new_state( const std::string& statefilename,
		  const std::vector<std::string>& varnames,
		  const std::vector<float64_t>& mins,
		  const std::vector<float64_t>& maxes,
		  const std::vector<std::string>& observation_varnames,
		  const std::vector<float64_t>& observation_stats,
		  float64_t epsil=1.0, //0.05,
		  int64_t maxgens=1e5,
		  int64_t numchains=40,
		  int64_t ndelta=3,
		  float64_t bnoise=0.05,
		  float64_t bstar=1e-6,
		  float64_t rthresh=1.2,
		  int64_t GRskip=50,
		  int64_t nCR=3,
		  int64_t pCRskip=10,
		  float64_t pjump=0.1,
		  int64_t M0d_mult=100,
		  int64_t Kthin=5 )
  {
    dream_abc_state::new_state( statefilename, varnames,
			       mins, maxes, observation_varnames,
			       observation_stats, epsil, maxgens,
			       numchains, ndelta, bnoise, bstar,
			       rthresh, GRskip, nCR, pCRskip, pjump );


    state.add_float64_matrix( Z_hist, varnames );
    state.add_int64_parameter( M0_param, varnames.size()*M0d_mult ); //REV: Need to fill Z with M0 at first.
    state.add_int64_parameter( K_Zthin_param, Kthin );
    state.add_int64_parameter( M_param, 0 );
  }

  
  
  //@OVERLOAD (added last step to thin Z with K)
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
    
    int64_t tgen = get_param<int64_t>(t_gen);
    int64_t crskip = get_param<int64_t>(pCR_skip_param );
    int64_t grskip = get_param<int64_t>(GR_skip_param );
    int64_t Kskip = get_param<int64_t>( K_Zthin_param );

    if( (tgen+1) % crskip == 0 )
      {
	update_pCR();
      }
    
    if( (tgen+1) % grskip == 0)
      {
	bool wouldconverge = compute_GR();
      }

    if( (tgen+1) % Kskip == 0 )
      {
	add_current_gen_to_Z();
      }
    
    END_GEN();
  }

  void add_current_gen_to_Z()
  {
    std::vector<std::vector<float64_t>> Xcurr = get_current_gen();
    state.add_to_matrix<float64_t>( Z_hist, state.get_varnames(Z_hist), Xcurr );
    return;
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

    //fprintf(stdout, "ABC: will choose dims and pairs\n");
    choose_moving_dims_and_npairs( pairidxs, movingdims, gamma, rand_gen );

    //fprintf(stdout, "ABC: DONE: choose dims and pairs. Will now get slices from matrix history\n");

    //std::vector<std::vector<float64_t> > mypairs = indices_to_vector_slices< std::vector<float64_t> >(Xcurr, pairidxs );
    //Draw from Z_hist

    std::vector< std::vector<float64_t> > mypairs = state.get_matrix_row_slice<float64_t>( Z_hist, pairidxs );
    
    //fprintf(stdout, "ABC: Got history slices\n");
    
    std::normal_distribution<float64_t> gdist(0, get_param<float64_t>( bstar_param ) );
    float64_t bwid = get_param<float64_t>( b_noise_param  );
    std::uniform_real_distribution<float64_t> udist(-bwid, bwid);
    
    //    fprintf(stdout, "ABC: Made rand distrs\n");
    
    std::vector< std::vector<float64_t> > dimdiffs(  mypairs.size()/2, std::vector<float64_t>(proposal.size(), 0 ) );
    std::vector<float64_t> dimdiffs_total( proposal.size(), 0);
    std::vector<float64_t> dimdiffs_wnoise( proposal.size(), 0);

    //fprintf(stdout, "ABC: Made dim diffs vects\n");
    
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
    
  
    proposal = edge_handling( proposal, rand_gen );
    
    
    return proposal;
  }

  std::vector<float64_t> edge_handling( const std::vector<float64_t>& proposal,
					std::default_random_engine& rand_gen )
  {
    //CLAMP seems best, but will stack up if it keeps trying to go out the side
    //REFLECT will give right "distribution", but bad position...
    //Random sample seems most "fair" but it will cause it to miss important (proposal) density that would otherwise cluster near the edge.
    std::vector<float64_t> newproposal = proposal;
    std::vector<float64_t> dim_mins = get_vector_param<float64_t>( dim_mins_param );
    std::vector<float64_t> dim_maxes = get_vector_param<float64_t>( dim_maxes_param );
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
	    newproposal[d] = dim_mins[d] + (dim_maxes[d] - dim_mins[d]) * udist( rand_gen );
	  }
      }
    
    return newproposal;
  }
  
  std::vector<std::vector< float64_t> > make_proposals(std::default_random_engine& rand_gen)
  {
    size_t nchains = get_param<int64_t>(N_chains_param);
    std::vector<std::vector<float64_t> > proposals;
    std::vector<std::vector<float64_t> > Xcurr = state.get_last_n_rows<float64_t>( X_hist, nchains );

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
  
  
  
  //REV: @OVERLOAD!!!
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


  //REV: @OVERLOAD (now calling different (overloaded) version of
  //draw_DE_pairs...
  //Other than that it is same).
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
    
                
    return;
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
  //Automatically call parent
 dream_abc_z_state()
   : dream_abc_state()
    {
      
    }
  
 dream_abc_z_state( const long& seed)
   : dream_abc_state( seed )
  {
    
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




