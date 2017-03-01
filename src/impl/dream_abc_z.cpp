#include <dream_abc_z.h>

  //@OVERLOAD
  void dream_abc_z_state::new_state ( const std::string& statefilename,
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
				      int64_t backupskip,
				      int64_t M0d_mult,
				      int64_t Kthin
		   )
		   
  {
    dream_abc_state::new_state( statefilename, varnames,
				mins, maxes, observation_varnames,
				observation_stats, observation_epsilons, maxgens,
				numchains, ndelta, bnoise, bstar,
				rthresh, GRskip, nCR, pCRskip, pjump,
				backupskip );


    state.add_float64_matrix( Z_hist, varnames );
    state.add_int64_parameter( M0_param, varnames.size()*M0d_mult ); //REV: Need to fill Z with M0 at first.
    state.add_int64_parameter( K_Zthin_param, Kthin );
    state.add_int64_parameter( M_param, 0 );
  }
  
  void dream_abc_z_state::add_current_gen_to_Z()
  {
    std::vector<std::vector<float64_t>> Xcurr = get_current_gen();
    state.add_to_matrix<float64_t>( Z_hist, state.get_varnames(Z_hist), Xcurr );
    size_t Mval = get_param<int64_t>( M_param );
    set_param( M_param, (Mval+Xcurr.size()) );
    return;
  }
  
  
  //@OVERLOAD
  void dream_abc_z_state::cleanup_gen()
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
  std::vector<std::vector<float64_t> > dream_abc_z_state::get_mypairs_vectors( const std::vector<size_t>& pairidxs )
  {
    std::vector<std::vector<float64_t> > mypairs = state.get_matrix_row_slice<float64_t>( Z_hist, pairidxs );
    return mypairs;
  }
  
  //@OVERLOAD
  std::vector<size_t> dream_abc_z_state::draw_DE_pairs( const size_t& npairs,
				     std::default_random_engine& rand_gen )
  {
    return choose_k_indices_from_N_no_replace( get_param<int64_t>( M_param ), npairs*2, rand_gen );
  }

  //@OVERLOAD
  //Now draw and add to Z, draw M0 from the hypercube?  BUt init pop is same.
  void dream_abc_z_state::generate_init_pop( std::default_random_engine& rand_gen, filesender& fs, parampoint_generator& pg )
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
  

  //OVERLOADED -- NO **MASKED*** i.e. different return type!!!
  //REV: UGLY UGLY UGLY PARSE SHARED STUFF BEFORE, NEED TO MAKE THIS BETTER LATER!!
dream_abc_z_state::abczconfig dream_abc_z_state::parseopts( optlist& opts )
  {
    dream_abc_z_state::abczconfig conf;

    
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
	fprintf(stdout, "DREAM ABC: User specified **RESTART** option. The previously created HDF5 Collection [%s] will be used for this search!!! Note: An additional argument can be added to this option, an integer value MAXGENS that specifies net MAXGENS. -MAXGENS parameter (and all other config parameters) will be ignored.\n", conf._statefilename.c_str());
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
	if( !conf.restart )
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

  
  
  //OVERLOADED? No, actually a different function...
  void dream_abc_z_state::create_with_config( dream_abc_z_state::abczconfig& c )
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
	    c.load_abcz_params( c._CONFIGFILE );
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
		   c._backupskip,
		   c._M0d_mult,
		   c._Kthin
		   );
      }
  }


  //CTOR
  //REV: **DO NOT CALL PARENT CONSTRUCTOR IT WILL BREAK THINGS B/C BASE CLASS FUNCTIONS CALLING OVERRIDDEN FUNCTIONS WILL NO LONGER CALL
  //THE OVERRIDDEN VERSION!!!! **
  
dream_abc_z_state::dream_abc_z_state( optlist& opts )
    {
      initialize();

      abczconfig c = parseopts( opts );

      create_with_config( c );
    }

  dream_abc_z_state::dream_abc_z_state()
    {
      initialize();
    }
  
  dream_abc_z_state::dream_abc_z_state( const long& seed)
    {
      initialize(seed);
    }
  


void search_dream_abc_z( optlist& opts,
			 parampoint_generator& pg,
			 filesender& fs
			 )
{
  dream_abc_z_state state( opts );
  
  state.run( fs, pg );

  return;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////////////////////////////////////////////////// OLD VERSION


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

  const float64_t epsil=0.05;
  std::vector<float64_t> epsils( observed_data.size(), epsil );
  
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




