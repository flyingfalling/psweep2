//REV: header file for searcher, which provides various search functions
//User will call one of these, passing the appropriate inputs,
//and it will automatically construct everything and run the sweep I guess....




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

#pragma once

#include <filesender.h>
#include <searchalgos.h>

#include <iterator>     // ostream_operator
#include <boost/tokenizer.hpp>
#include <math.h>

#include <utility_functs.h>
#include <csvparser.h>


#include <dream_abc.h>
#include <dream_abc_z.h>
//#include <boost/cstdfloat.hpp>
//#include <stdfloat.h>
//#include <stdint.h>



//Globally, I want to have some struct to call it with? Like REGISTER_FUCNT. I don't want user to have to do stuff. I.e. make a "searcher" struct,
//much nicer. Then call run-search. OK.

struct searcher
{

  //Has a fake memsystem in there too hahaha...
  fake_system fakesys;

  parampoint_generator pg;
  
  searcher()
  {
    //REV: nothing todo
  }
  
  void register_funct( const std::string& name, const fake_system_funct_t& funct)
  {
    fakesys.register_funct( name, funct );
  }
  
  //varlist will contain required um, data files I guess?
  void run_search( const std::string& searchtype, const std::string& scriptfname, const std::string& mydir, /*const*/ varlist<std::string>& params, const bool& writefiles=false )
  {
    pg = parampoint_generator(scriptfname, mydir);

    fprintf(stdout, "REV: Finished making parampoint generator, now will create FILESENDER\n");
    //REV: PG contains the "results" of each... parampoint_results, of type parampoint_result.
    //That is: list of pset results, each of which has list of pitem results (specifically, varlist).
    //OK, I can access those however I wish, e.g. I know last is the only one I care about etc.

    //REV: User must have created his FAKESYSTEM calls before this point. In other words, in user program, he makes his main, he has his funct,
    //he registers his funct, then when he calls this, he calls it with his list of his FAKE_SYSTEM stuff. OK.


    //Can a static funct take an argument...? I guess so.
    filesender* fs = filesender::Create( fakesys, writefiles );
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
	search_grid( varnames, mins, maxes, steps, pg, *fs);
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
    std::string contents="EXIT";
    boost::mpi::broadcast(fs->world, contents, 0);
  
    delete(fs);
  
  }


}; //end struct searcher
