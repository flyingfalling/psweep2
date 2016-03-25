

//REV: header file for searcher, which provides various search functions
//User will call one of these, passing the appropriate inputs,
//and it will automatically construct everything and run the sweep I guess....


//REV: TOdo, need to make it so that RESTART will automatically find/choose correct algo. In other words, dont need to tell it type for it to load...
//So, we need to store the algo in the state file (duh..).



//takes user options and runs corresponding search funct?

//"Format" is required...for each different search, what kind of info it needs to parameterize it. I guess just parameterize it with a global varlist.
//REV: OK how to parameterize it with some global varlist?
//Do like PARAMNAME, PARAMMIN, PARAMMAX? How to name it? In individual varlists?
//Use varlists NAMES to find it, inside what? A hierarchical? No...just use a varlist structure... I.e. named list of varlists...Ugh.
//This seems "best", but then user needs to be careful to construct his scripts/etc. as max/min/etc. Or give a better way to read varlists,
//like VAL MIN MAX STEP etc.? So, basically ways of taking 2-d values? Need to take "column" headers, etc. Need it for simulation anyway (read mii-sans
//data?). OK, I guess I can do this. So, now we have some kind of 2d var list structure, where we tell it the variable (row#?) and the column name
// (e.g. var name vs val vs etc.?)
//Effectively we have a 2-column thing now. Make it even more general? We want to eventually have more "abstract" i.e. jagged ones, as we talked about
//before...i.e. nested information.
//I.e. varlists are arbitrary? but then how do we hold arrays? In varlist, it's just an arbitrary list until some endthing? How do I know where it
//ends in read-in? That is the problem?
//Need a way to have arbitrary length array stored somewhere? Only up to 3D array? Either 1D of array type (i.e. all guys are arrays), or 2D of single
//value types...? Need a way to arbitrarily "flatten" matrices of arbitrary length. Ways of storing in memory. Of course, we can but reading/writing
//will not be compressed/efficient. If it includes strings, that will make it only more complex.
//But in the eventuality that I want to run experiments/store it on NSIM side, need a way to do it, right? I don't want to have to repeat every single
//variable every single time. E.g., better to have a way to compress it uniquely. But representation will change if I change number of guys.
//E.g.

//We can rearrange this in some way to make it "hierarchical" based on most efficient graph/tree search... i.e. might be more efficient to "list" it
//by timepoints (var4) if there are more (fewer?) of those. But then how to arbitrarily do it without actually listing out every single one, e.g.
//tell it what each "location" is and then store it that way. I.e. don't write "neuron#" or "time point" each time., just do it? HDF5 might be the
//most elegant way of doing this? But how do I go through and analyse that? E.g. electrode # etc. Doing e.g. EL1 and EL2, or some arbitrary number of
//electrodes (and their positions). How to list those out then? Like, how to specify "experiment" to do most efficiently?
//I.e. TIMING, LOCATION, STRENGTH of each stimulation or something like that. Need a way to parameterize that in an N dimensional space. Timing of
//each is a dimension? Similar to eye movement/visual stimulation analysis. Too many dimensions, and not clear how to orient them to make it most
//informative. Like, I want to basically "list" parameters in some hierarhical way, and finally have e.g. TIMEPOINT and NEURONNUMBER known without
//listing them. Need a "organization" file (how to interpret data file), and then "data file" itself. Same file is obviously best. HDF5?
//Anyway, in this case, do named varlists obviously, easiest. Wasteful. Want to have 2d table things to read too though. Use HDF5...exists, and
//efficient. PRoblem is that stored information might have arbitrary lengths (i.e. strings), or might be doubles or whatever. But to process what
//user is doing, he will make it via some CMD program? Some GUI?

// VAR1 VAR2 VAR3 VAR4 THING
//  1    1     1    1    2.5
//  1    1     1    2    3.5
//  1    1     1    3    95.9
//  1    1     2    1    2.5
//  1    1     2    2    3.5

//For now just pass MIN MAX etc.. Yea, way to handle 2d after all. It contains..? How about going from HDF5 to?
//Hm, this is kind of nasty...because when I "extracted" each column, I would need to appropriately have the return function return
//the kind, or cast it manually each time -_-

#include <searcher.h>

//REV: Where to set parameters?
//We can do it "above" to handle options, but that makes it a pain. Assume all variables are already set and I just "check" for them?
//Inside the algorithm itself?
//Another option is to have a set "config" file for running the sweep. For example "load" etc.? I don't want to specify (on command line?)
//The arguments each time I start a run? Or do I? I want it to be "stored". It is stored, in the file itself of course.

//Make an error if user "doubly defines" things. E.g. allow him to set a "config" file for the search algorithm, or a "default", but then also add
//cmd line arguments. Give a warning (error/exit?) if cmnd line overrides confnig file, or config overrides config,e tc.


//REV: This is the thing that is compiled into the library! I.e. globally accessible is this!
//I need to add everything to CPP files so that I can separately compile them...and do it faster...
//Let's do it now I guess? They are all including each other though, which causes some problems.

searcher::searcher( )
  {
    //REV: Nothing to do
  }

void searcher::register_funct( const std::string& name, const fake_system_funct_t& funct)
  {
    fakesys.register_funct( name, funct );
  }

void searcher::run_search( optlist& opts )
  {
    //parse to required guys that I want... ONLY ROOT RANK SHOULD EXECUTE THIS, CRAP.
    preparseopts( opts );
    
    filesender* fs = filesender::Create( _runtag , fakesys, _wrkperrank, _writefiles );
    
    opts.enumerateparsed();
    opts.enumerateextras();
    
    parseopts( opts ); //Could just get individual things like GETSEARCHTYPE, etc. To reduce "fake" internal members we don't need...
    
    //REV: Moved this up here to save user from doing it...
        
    //But this way at least we are kind of "explicit" about what we consume..?
    run_search( _searchtype, _scriptfname, _mydir, opts, *fs, _writefiles );
  }


  //REV: Faster to specify some struct to handle all options, this way it can easily know how many args it wants, and what are usage things so that
  //they can be printed...

void searcher::preparseopts( optlist& opts )
  {
    //set internal variables with parseopts
    auto a = opts.get_opt_args( "WRITEFILES" );
    if( a.size() == 0 )
      {
	//fprintf(stdout, "SEARHER: Parseopts, -WRITEFILES **NOT** defined. Will *not* write files to filesystem (I.e. will use memfsys)\n");
	_writefiles = false;
      }
    else
      {
	//fprintf(stdout, "SEARHER: Parseopts, -WRITEFILES defined, *will* write files to filesystem (I.e. will not use memfsys)\n");
	_writefiles = true;
      }

    //set internal variables with parseopts
    a = opts.get_opt_args( "WORKERSPERRANK" );
    if( a.size() == 0 )
      {
	//defaulting to 1.
	_wrkperrank=1;
	fprintf(stdout, "REV: found no cmd line arg, so SETTING WORKERS PER RANK TO [%ld]\n", _wrkperrank);

      }
    else
      {
	if( a[0].size() > 0 )
	  {
	    _wrkperrank = std::stol(a[0][0]);
	    fprintf(stdout, "REV: SETTING WORKERS PER RANK TO [%ld]\n", _wrkperrank);
	  }
	else
	  {
	    fprintf(stderr, "ERROR. SEARCHER in option WORKERSPERRANK: Requires at least 1 argument (*#WORKERS PER RANK*)\n");
	    exit(1);
	  }
      }

    //set internal variables with parseopts
    a = opts.get_opt_args( "TAG" );
    if( a.size() == 0 )
      {
	//We have no tag. We use the default.
	//fprintf(stdout, "SEARHER: Parseopts, -WRITEFILES **NOT** defined. Will *not* write files to filesystem (I.e. will use memfsys)\n");
	//_writefiles = false;
      }
    else
      {
	if( a[0].size() > 0 )
	  {
	    _runtag = a[0][0];
	  }
	else
	  {
	    fprintf(stderr, "ERROR. SEARCHER in option TAG: Requires at least 1 argument (*TAG* of this run, for scratch naming purposes)\n");
	    exit(1);
	  }
	//fprintf(stdout, "SEARHER: Parseopts, -WRITEFILES defined, *will* write files to filesystem (I.e. will not use memfsys)\n");
	//_writefiles = true;
      }
   
    
  }
  
void searcher::parseopts( optlist& opts )
  {
    //set internal variables with parseopts
    auto a = opts.get_opt_args( "WRITEFILES" );
    if( a.size() == 0 )
      {
	fprintf(stdout, "SEARHER: Parseopts, -WRITEFILES **NOT** defined. Will *not* write files to filesystem (I.e. will use memfsys)\n");
	_writefiles = false;
      }
    else
      {
	fprintf(stdout, "SEARHER: Parseopts, -WRITEFILES defined, *will* write files to filesystem (I.e. will not use memfsys)\n");
	_writefiles = true;
      }
    
    auto b = opts.get_opt_args( "DIR" );
    if( b.size() == 0)
      {
	fprintf(stderr, "ERROR SEARHER: Parseopts, did *not* specify a -DIR for running, please specify and run again\n");
	exit(1);
      }
    else
      {
	if(b[0].size() == 0 || b[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR SEARCHER: Parseopts, -DIR option had [%ld] arguments, expects only 1 (name of dir)\n", b[0].size() );
	    exit(1);
	  }
	else
	  {
	    _mydir = b[0][0];
	  }
	fprintf(stdout, "SEARCHER: Using specified DIR [%s] as DIR for running search\n", _mydir.c_str());
	}

    auto c = opts.get_opt_args( "SEARCHTYPE" );
    if( c.size() == 0 )
      {
      	fprintf(stderr, "ERROR SEARHER: Parseopts, did *not* specify a -SEARCHTYPE for running, please specify and run again\n");
	exit(1);
      }
    else
      {
	if(c[0].size() == 0 || c[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR SEARCHER: Parseopts, -SEARCHTYPE option had [%ld] arguments, expects only 1 (searchtype)\n", c[0].size() );
	    exit(1);
	  }
	else
	  {
	    _searchtype = c[0][0];
	  }
	fprintf(stdout, "SEARCHER: Using specified SEARCHTYPE [%s] for running search\n", _searchtype.c_str() );
      }

    auto d = opts.get_opt_args( "WORKSCRIPT" );
    if( d.size() == 0 )
      {
      	fprintf(stderr, "ERROR SEARHER: Parseopts, did *not* specify a -WORKSCRIPT for running, please specify and run again\n");
	exit(1);
      }
    else
      {
	if(d[0].size() == 0 || d[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR SEARCHER: Parseopts, -WORKSCRIPT option had [%ld] arguments, expects only 1 (searchtype)\n", d[0].size() );
	    exit(1);
	  }
	else
	  {
	    _scriptfname = d[0][0];
	  }
	fprintf(stdout, "SEARCHER: Using specified WORKSCRIPT [%s] for running search\n", _scriptfname.c_str() );
      }
       
  }

  //varlist will contain required um, data files I guess?
void searcher::run_search( const std::string& searchtype, const std::string& scriptfname,
		   const std::string& mydir, optlist& opts,
		   filesender& fs,
			   const bool& writefiles )
  {

    fprintf( stdout, "RUNNING SEARCH WITH: searcytype [%s], scriptfname [%s], mydir [%s]\n", searchtype.c_str(), scriptfname.c_str(), mydir.c_str() );
    
    std::vector<std::string> registeredstypes = { "GRID",
						  "DREAM-ABC",
						  "DREAM-ABCz" };
      //"MT-DREAMz" };

    auto locs = find_string_in_vect( searchtype, registeredstypes );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR, requested search type [%s] is not implemented/not available. Valid types:\n", searchtype.c_str());
	//print1d_str_vec_row( registeredstypes );
	for(size_t x=0; x<registeredstypes.size(); ++x)
	  {
	    fprintf(stderr, "[%s] ", registeredstypes[x].c_str());
	  }
	fprintf(stderr, "\n");
	exit(1);
      }
    
    pg = parampoint_generator(scriptfname, mydir);
    
    //fprintf(stdout, "REV: Finished making parampoint generator, now will create FILESENDER (this will cause MPI ranks to initialize!!!!)\n");
    
    
    if( searchtype.compare( "GRID" ) == 0 )
      {
	//pass as options...
	search_grid( opts, pg, fs );
	
      }
    else if( searchtype.compare( "DREAM-ABC") == 0 )
      {
	//pass as options...
	search_dream_abc( opts, pg, fs );
	
      }
    else if( searchtype.compare( "DREAM-ABCz") == 0 )
      {
	search_dream_abc_z( opts, pg, fs );
      }
    else
      {
	fprintf(stderr, "ERROR: searcher, not recognized search type [%s] (Or I made a misstype in the if/else! Sorry!)\n", searchtype.c_str());
	doexit( &fs );
	exit(1);
      }
    
    doexit( &fs );
  }

void searcher::doexit( filesender* myfs )
  {
    fprintf(stderr, "ROOT FINISHED! Broadcasting EXIT (in SEARCHER.CPP)\n");
    //std::string contents="EXIT";
    //boost::mpi::broadcast(myfs->world, contents, 0);
    //This won't work with worker threads. Need to iter through WORKERS.
    
    myfs->signal_exit_to_workers();
    
    
    delete(myfs);
  }



  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////// OLD RUN SEARCH
  
  //varlist will contain required um, data files I guess?
void searcher::run_search( const std::string& searchtype, const std::string& scriptfname, const std::string& mydir, /*const*/ varlist<std::string>& params, const bool& writefiles )
  {
    
    pg = parampoint_generator(scriptfname, mydir);

    fprintf(stdout, "REV: Finished making parampoint generator, now will create FILESENDER\n");
    //REV: PG contains the "results" of each... parampoint_results, of type parampoint_result.
    //That is: list of pset results, each of which has list of pitem results (specifically, varlist).
    //OK, I can access those however I wish, e.g. I know last is the only one I care about etc.

    //REV: User must have created his FAKESYSTEM calls before this point. In other words, in user program, he makes his main, he has his funct,
    //he registers his funct, then when he calls this, he calls it with his list of his FAKE_SYSTEM stuff. OK.


    //Can a static funct take an argument...? I guess so.
    filesender* fs = filesender::Create( _runtag, fakesys, writefiles, 1 );
    //REV: Oh crap, on this side, it might need to read them in the first place...hm.


    //If we want master but not slaves to be different than writefiles, do it here...
    
    //Calls to e.g. SEARCH_GRID etc. will call it. All calls to FS that master will make, I need to make sure to handle them properly...
    
    fprintf(stdout, "RUNNING SEARCH ALGO: [%s]\n", searchtype.c_str() );
  
    if( searchtype.compare( "grid" ) == 0 )
      {
      
	std::string varname = "GRID_MIN_MAX_STEP_FILE";
	std::string minmaxfname = params.getTvar( varname );
      
	bool hascolnames = true;
	data_table dtable( minmaxfname, hascolnames );

	fprintf(stdout, "Trying to get VARNAMEs\n");
	std::vector<std::string> varnames = dtable.get_col( "NAME" );

	fprintf(stdout, "Got varnames\n");
	std::vector<double> mins = data_table::to_float64( dtable.get_col( "MIN" ) );
	fprintf(stdout, "Got mins\n");
	std::vector<double> maxes = data_table::to_float64( dtable.get_col( "MAX" ) );
	std::vector<double> steps = data_table::to_float64( dtable.get_col( "STEP" ) );
	
	fprintf(stdout, "Got STEP\n");
      
	//Construct required stuff from PARAMS. I.e. min and max of each param? Need N varlists? Have them named? Specific name? Have array type of
	//name PARAMS, etc.? Probably got from a file at beginning... Some special way of reading that...it should know varnames? 
	grid_state gs;
	gs.search_grid( varnames, mins, maxes, steps, pg, *fs);
      }


    else if( searchtype.compare( "DREAM-ABC" ) == 0 )
      {
	std::string varname = "ABC_TEST_MIN_MAX_FILE";
	std::string minmaxfname = params.getTvar( varname );

	std::string obsdatafname = "ABC_TEST_OBSERV_DATA_FILE";
	std::string observfname = params.getTvar( obsdatafname );
	
	
	bool hascolnames = true;
	data_table dtable( minmaxfname, hascolnames );
	data_table obsvdtable( observfname, hascolnames );
	
	fprintf(stdout, "Trying to get VARNAMEs\n");
	std::vector<std::string> varnames = dtable.get_col( "NAME" );
	
	fprintf(stdout, "Got varnames\n");
	std::vector<double> mins = data_table::to_float64( dtable.get_col( "MIN" ) );
	fprintf(stdout, "Got mins\n");
	std::vector<double> maxes = data_table::to_float64( dtable.get_col( "MAX" ) );
	fprintf(stdout, "Got maxes\n");
	
	std::string statefname = "dreamsearch_state.state";
	
	//Make a random "problem"
	size_t ndims = varnames.size();


	fprintf(stdout, "Getting observ data from [%s]\n", observfname.c_str() );
	std::vector<std::string> obsv_varnames = obsvdtable.get_col( "NAME" );
	std::vector<double> obsv_vals = data_table::to_float64( obsvdtable.get_col( "VAL" ) ); //REV: this will just be ERROR and 0 for me... heh.

	//Load or run? Do it here or elsewhere? Do it in ABC?
	search_dream_abc( statefname,
			  varnames,
			  mins,
			  maxes,
			  obsv_varnames,
			  obsv_vals,
			  pg,
			  *fs
			  );
      }

    
    else if( searchtype.compare( "DREAM-ABCz" ) == 0 )
      {

	//DREAM-ABCz expects:
	//1) ABC TEST MIN/MAX file which has var namesand values
	//   e.g. list of #d: name min max values.
	//   Has headers: NAME MIN MAX
	//2) ABC TEST OBSERV DATA FILE, which is filename of file
	//   containing observations from data (Y vector)
	//   Has headers: NAME
	//  
	std::string varname = "ABC_TEST_MIN_MAX_FILE";
	std::string minmaxfname = params.getTvar( varname );

	std::string obsdatafname = "ABC_TEST_OBSERV_DATA_FILE";
	std::string observfname = params.getTvar( obsdatafname );
	
	
	bool hascolnames = true;
	data_table dtable( minmaxfname, hascolnames );
	data_table obsvdtable( observfname, hascolnames );
	
	fprintf(stdout, "Trying to get VARNAMEs\n");
	std::vector<std::string> varnames = dtable.get_col( "NAME" );
	
	fprintf(stdout, "Got varnames\n");
	std::vector<double> mins = data_table::to_float64( dtable.get_col( "MIN" ) );
	fprintf(stdout, "Got mins\n");
	std::vector<double> maxes = data_table::to_float64( dtable.get_col( "MAX" ) );
	fprintf(stdout, "Got maxes\n");
	
	std::string statefname = "dreamsearch_state.state";
	
	//Make a random "problem"
	size_t ndims = varnames.size();


	fprintf(stdout, "Getting observ data from [%s]\n", observfname.c_str() );
	std::vector<std::string> obsv_varnames = obsvdtable.get_col( "NAME" );
	std::vector<double> obsv_vals = data_table::to_float64( obsvdtable.get_col( "VAL" ) ); //REV: this will just be ERROR and 0 for me... heh.

	//Using that, I search.

	//I need a way to pass more variables further in, by passing
	//a "named" varlist for example, which DREAM_ABC_Z struct
	//knows how to handle natively.

	//This "search" funct thing calls with no arguments
	//(kind of a problem). However, I'd like to be able to change
	//things like observations part way through? No...
	//Or only copy certain aspectds like current positions to
	//a new sweep?

	//Whatever, just add everything to a "command like" processor,
	//which then goes to a "named" varlist.
	//The "named" varlist is not hierarchical? We want a "named"
	//hierarchical varlist, which has variables for model, under
	//a general one with e.g. numthreads, etc. Workers need to
	//know # of GPU it's working on etc. Worker only reports when
	//GPU is ready? Does psweep2 *know* about GPUs?
	//I can query GPUs on machine and only grab those of certain
	//type I want (?). Ideal situation is to keep most "code"
	//loaded on GPU, and only modulate state variables...
	
	search_dream_abc_z( statefname,
			    varnames,
			    mins,
			    maxes,
			    obsv_varnames,
			    obsv_vals,
			    pg,
			    *fs
			    );
      }
    else if( searchtype.compare( "MT-DREAMz" ) == 0 )
      {
	fprintf(stderr, "REV: Error, requested search algo MT-DREAMz is not implemented yet!\n");
	exit(1);
      }
    else
      {
	fprintf(stderr, "REV: ERROR, search algorithm type [%s] not found\n", searchtype.c_str() );
      }
  
    fprintf(stderr, "ROOT FINISHED! Broadcasting EXIT\n");
    //std::string contents="EXIT";
    //boost::mpi::broadcast(fs->world, contents, 0);
    fs->signal_exit_to_workers();
    
    delete(fs);
  
  }

