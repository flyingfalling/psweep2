//REV: add some "CMDS" to execute at beginning, e.g. MAKE all guys etc.
//May set some global variables? Locate data files? CP data files?
//Figure that out later ;)

//We can keep all variables internal? Nasty...

//NOTE: REQUIRED files may need to be copied locally?!?!?! Or should I just access them elsewher? I will not modify them, only read from them I assume...
//Individual PSETS may (before threading) have "shared" parameters file! And other files? And these are copied to all worker???

//Have ways to access PSET specific files and, data files like sound files etc.? Should all such files be copied to "current worker"? Obviously we don't
//want to keep them around afterwards. But, have user specify them all at beginning so we can store them all in some "data source" for this "run". That
//way there is no double.
//We can also e.g. require whole directories, like C++ source files etc., for posterity. Obviously we can't do libs etc., but for user models, we can
//have them force the source code in, and have it auto-compile etc. at beginning. And even check some hash of it regularly to make sure there are
//no changes heh. Or at least give a warning. I.e. for all executables or scripts we will be running.

//REV: in the "hierarch" varlist will the param settings be stored? Are they always single variable single value? I guess.
//Kind of stupid to "force" it to store. Make a function that does the writing and then require that file I guess ;)
//but, in that case I need a way to write other "set" variables to the file?


//REV: what do I need to do to get a FUNCTIONAL thing???
//I.e. best to keep all "variable" files stored in memory???!


//Difference between "workers" and "non-workers". Note, for each (worker) guy, there will be a specific "data" holding folder that holds it for
//that worker. It may be in TMP. These can be specified as general data guys required for everyone. I guess we could have separate for each WORKER
//since it may need to be held locally. Or even faster, just once for each NODE, if we can logically include that.


//Right now, I can do everything...but how do I get the variables? Yappa, specify all "required" files. How do I get access to "write out" param for
//passing to user? Make it an option? Have a list of "fixed" variables always present too? Write them all out every time? Or leave them separate.
//Best is to have that "option". I.e. specify "fixed" params, etc. But, of course it doesn't know that. Still need a way for me to get my params to
//user program ostensibly. So, let user set a variable telling file to which to write params? I.e. "input filename", we don't know content though?

//Force user to store it that way though? Var-value. Can variables (read in by user?) be arrays as well?


//Automatically write it to a specified guy after executing of script (user might set it in there).

//REV: build TEST to actually execute some simple guy, build the dirs, set variables appropriately, read out results, etc.

//Test a couple cases where test program does/doesn't do stuff.

//Set a "chunk size" of parampoints that I save HDF5 after.

//TODO, need to make it so that I can actually construct CMD out of CMD added things.

//REV: todo, specify "shared" data points (so it doesn't copy them). We can do that just fine.
//There must be some "basic" required dirs for the whole thing to run in the first place. It will check all of that.
//So, those are all copied at beginning (?).

//Separate "DATA" required files etc.? From other types of required files? Like large guys. We don't want to re-copy each time? But, each one
//should have a copy? But for very large things, passing in memory is obviously much better orz. Reading the same file over and over is bullshit.


//REV: set everything up to set up directories now. In other words, order them appropriately and name them in that way.
//And automatically add the correct "existing" guys to the variable list etc.

//WRITE FUNCTS:
// varlist.inputfromfile( filename ) //adds vars in that file to this varlist.
// bool check_file_existence( fname )


//## TODO: make special functs to add file to REQUIRED/SUCCESS/etc.
//TODO: Make it so that we can also pass along/check "named" varlist
//TODO: make special functs to access previous psets/pitems (directories) so we can do that. It will get like "dir" of that one? Or something?
//TODO: Figure out how to organize directories/access them? Workers/etc.

//Questions: how to deal with variables over different PSETS in PARAMPOINT
//Questions: how to build CMD better? Add option type thing? Make CMD a str var array and have it do the stuff ;). And at the end it adds with seps
//           or something

//TODO: special variables to generate guys from array with other string, with certain sep. E.g. to access "all" the other worker guys. Need a way to
//      access programmatically how many PARALLEL guys there are? Yappa over PSET we want to have many. Have a special function to access #P or something.

//REV: names...um, need to have way to deal with different models names/specific variables. That is what the named variables are.

#pragma once

#include <functrep.h>
#include <utility_functs.h>

//REV: this is a parampoint. It has a command thing that is what to run
//each time point. Note, we only actually want one of those representations,
//that is run each time I guess, applied to the specific parampoint.

//The parampoints need to have their variable lists or something. I'd really prefer to not have a global var list. But, we can always just pass it
//as an argument in functions.

std::string CONCATENATE_STR_ARRAY(const std::vector<std::string>& arr, const std::string& sep)
{
  std::string ret="";
  for(size_t a=0; a<arr.size(); ++a)
    {
      ret+= arr[a] + sep;
    }
  return ret;
}

//REV: this is a worker item. I.e. a single parameterized worker thing.
struct pitem
{
  std::string mycmd; //the main thing, this is the cmd I am running via system()?
  
  std::vector< std::string > required_files;
  std::vector< std::string > success_files;

  std::vector< std::string > output_files;

  std::string input_file;
  
  std::string mydir;
  
  size_t my_hierarchical_idx; //index in my parampoint hierarchical varlist array of my leaf node.

  //In new version, this may call from memory to speed things up.
  bool execute_cmd( )
  {
    std::vector< std::string > notready = check_ready();
    if( !notready.size() > 0 )
      {
	fprintf(stderr, "REV: Error in executing of command:\n[%s]\n", mycmd.c_str());
	fprintf(stderr, "found [%ld] NON-EXISTENT REQUIRED FILES: ", notready.size() );
	for(size_t f=0; f<notready.size(); ++f)
	  {
	    fprintf(stderr, "FILE: [%s]\n", notready[f].c_str());
	  }
	exit(1);
      }
    
    system( mycmd );
    
    std::vector<std::string> notdone = checkdone();

    size_t tries=0;
    size_t NTRIES=10;
    while( notdone.size() > 0 && tries < NTRIES)
      {
	system( mycmd ); //Delete/reset this pitem?
	notdone = checkdone();
      }

    if(notdone.size() > 0)
      {
	fprintf(stderr, "REV: Error in executing of command:\n[%s]\n", mycmd.c_str());
	fprintf(stderr, "found [%ld] NON-EXISTENT SUCCESS FILES: ", notdone.size() );
	for(size_t f=0; f<notdone.size(); ++f)
	  {
	    fprintf(stderr, "FILE: [%s]\n", notdone[f].c_str());
	  }
	exit(1);
      }

    //More elegantly exit. Note all guys should output to a certain specific output file!!! In their dir.
    
    return done;
    //this only tells if it was SUCCESSFUL or not (?). For example we had problems before due to something failing for one reason or another. In this way
    //we can automatically restart it...?
  }
    

  void reconstruct_cmd_with_file_corresp( const std::vector< std::string >& orig, const std::vector< std::string >& new)
  {
    //Rebuild CMD array list, but replace all instances of ORIG with NEW.
    //Note if it was a DATA file, it will already exist with same name in data folder.
    
     
  }
  
  pitem( pset_functional_representation& pfr, const size_t idx,  hierarchical_varlist& hv)
  {
    //Need to add to the most recent pset a child...
    std::vector<size_t> rootchildren = hv.get_children( 0 );
    size_t npsets = rootchildren.size();
    size_t myidx = hv.add_child( rootchildren[npsets-1] ); //add child to the last one.
    my_hierarchical_idx = myidx;

    //Set up the required variables. Automatically name the things here (like choose dir based on sub of parent), much easier ;)

    std::string parentdir = hv.vl[ rootchildren[npsets-1] ].getvar( "__MY_DIR" ).get_s();
    mydir = parentdir + "/" + std::to_string( idx ); //REV: whatever, to_string shouldn't cause a problem here, other than not padding zeroes.
    
    
    variable<std::string> var1( "__MY_DIR", mydir );

    std::vector<std::string> emptyvect;
    variable<std::string> var2( "__MY_REQUIRED_FILES", emptyvect );
    variable<std::string> var3( "__MY_SUCCESS_FILES", emptyvect );
    variable<std::string> var4( "__MY_OUTPUT_FILES", emptyvect );
    variable<std::string> var5( "__MY_CMD", emptyvect );
    hv.vl[myidx].addvar( var1 );
    hv.vl[myidx].addvar( var2 );
    hv.vl[myidx].addvar( var3 );
    hv.vl[myidx].addvar( var4 );
    hv.vl[myidx].addvar( var5 );

    //Reserve __ vars internally or something? I'm not doing unrolling so whatever I guess.
    
    
        
    
    //Each must have for each worker the "required" files etc. Then it creates stuff and 
    bool succ = make_directory( mydir );
    
    
    std::vector<hierarchical_varlist> hvl;
    hvl.push_back( hv ); //will it modify it? I'm not sure! Crap.
    //This is a copy operator, so it will not bubble up the copy operator
    //unless I re-copy
    //Add other HVarlist to it such as the global NAMES one.
    
    //And now execute all arguments in pset_functional_rep
    for(size_t s=0; s<pfr.frlist.size(); ++s)
      {
	myvar_t retval = pfr.frlist[s].execute( hvl, myidx );
	//What to do with retval?!?!?! Ignore it?
      }

    //ghetto hack. GLOBAL namespace doesn't exist.
    hv = hvl[0];
    
    required_files = hv.get_array_var( "__MY_REQUIRED_FILES", my_hierarchical_idx );
    success_files = hv.get_array_var( "__MY_SUCCESS_FILES", my_hierarchical_idx );
    
    std::vector<std::string> cmdarray = hv.get_array_var( "__MY_CMD", my_hierarchical_idx );
    
    //Add output to correct file.
    std::string stderrfile = mydir+"/stderr";
    std::string stdoutfile = mydir+"/stdout";
    
    mycmd.push_back( "1>"+stdoutfile );
    mycmd.push_back( "2>"+stderrfile );
    
    std::string sep = " "; //spaces. Make user specify "cmd sep" if it exists or something?
    mycmd = CONCATENATE_STR_ARRAY( cmdarray, sep ); //hv.get_val_var( "__MY_CMD", my_hierarchical_idx );
    
    output_files = hv.get_array_var( "__MY_OUTPUT_FILES", my_hierarchical_idx );
    
    
    //REV: I need to set INPUT files too, which I will write to from what? From HVL I guess? OK...
    //Will it do all of them, or only the first one?
    //We need a "setup" way or something. Note this is for every PITEM. I.e. it will re-write it each time, which is not good
    //Maybe only the FIRST one needs it. OK.
    
    input_file = hv.get_val_var( "__MY_INPUT_FILE", my_hierarchical_idx );
    
    hv.tofile( input_file, my_hierarchical_idx ); //HAVE TO DO THIS HERE BECAUSE I NEED ACCESS TO THE HV. I could do it at top level though...
    
    //Input files are REQUIRED files (by default, might be checked twice oh well).
    //Furthermore, output files are SUCCESS files (fails and tries to re-run without their creation of course).
    required_files.push_back( input_file );
    for(size_t o=0; o<output_files.size(); ++o)
      {
	success_files.push_back( output_files[o] );
      }
    //but then I need to do it for each ugh.
    //REV: note this will write EVERYTHING in here out, which may not be needed/might mess things up.
    //Can we specify only things that are tagged for this model or something?
    //Just do all for now, hope it doesn't break lol.
    
    //mydir = hv.vl[my_hierarchical_idx].getvar("MY_DIR").get_s();
    
        
    //Finished constructing pitem, i.e. setting all variables.
    //Now we go through and use those to set REQUIRED and SUCCESS files
    //and MYCMD, and MYDIR, etc.
    //From our specific SPECIAL VARIABLES. May be more than 1? Or will
    //be an array?
    //MM yea, set zero length array SUCCESS and REQUIRED from beginning
    //and also (empty) CMD string variable? OK, those are "set". Also, like
    //MYDIR. Functions will return those variables.
    //Need a way like "get_success_variable_name" or something? User
    //may want to SET them. OK.

    //I.e. at the end of this these REAL variables will be set.

    //We need ways of finding inside GLOBAL vars named var etc.
  }
  
 
  bool checkready()
  {
    bool ready=true;
    //Strings must be FULL filename? Or are they relative? Assume full...
    for(size_t f=0; f<required_files.size(); ++f)
      {
	if( check_file_existence( required_files[f] ) == false )
	  {
	    ready = false;
	  }
      }
    return ready;
    
    //checks if all required files are present?? etc.
  }
  
  bool checkdone()
  {
    bool ready=true;
    //Strings must be FULL filename? Or are they relative? Assume full...
    for(size_t f=0; f<success_files.size(); ++f)
      {
	if( check_file_existence( success_files[f] ) == false )
	  {
	    ready = false;
	  }
      }
    return ready;
    
    //checks if I'm done. Specifically by seeing if all required guys are
    //finished. I need multiple "variable lists" of guys, some that
    //set required guys for starting, some that check required files for
    //ending. Don't check file contents though.
  }

  varlist get_output()
  {
    varlist retvarlist;
    
    //REV: I could read this some other way? Some number of (named) varlists? A list of them? OK.
    for(size_t f=0; f<output_files.size(); ++f)
      {
	retvarlist.inputfromfile( output_files[f] );
      }
    return retvarlist;
  }

  std::string get_cmd()
  {
    return mycmd;
  }
  
  
};


//REV:
struct pset
{
  std::string mydir;
  std::vector< pitem > pitems;
  size_t my_hierarchical_idx; //index in my parampoint hierarchical varlist array of my leaf (or middle) node.

  
  pitem farm_next_pitem()
  {
    for(size_t p=0; p<pitems.size(); ++p)
      {
	if( pitems[p].checkfarmed() == false )
	  {
	    pitems[p].markfarmed();
	    return pitems[p];
	  }
      }
    fprintf(stderr, "ERROR should not reach this point in pitem farm next pitem\n");
    exit(1);
  }

  bool checkfarmed()
  {
    //could make this have state var too but whatever...
    bool allfarmed=true;
    for(size_t p=0; p<pitems.size(); ++p)
      {
	if(pitems[p].checkfarmed() == false)
	  {
	    return false;
	  }
      }
    return allfarmed;
  }
  
  bool checkdone()
  {
    if(done == true)
      {
	return done;
      }
    else
      {
	done = true;
	//checks if this pset is done. Specifically, by checking if all (remaining) pitems are done! If not, wait until they return.
	for(size_t pi=0; pi<pitems.size(); ++pi)
	  {
	    if( pitems[pi].checkdone() == false )
	      {
		done=false;
		return done;
	      }
	  }
      }
    fprintf(stderr, "ERROR in checkdone in pset, why am I reaching end?\n"); exit(1);
  }


  pset(hierarchical_varlist& hv)
  {
    //std::vector<size_t> rootchildren = hv.get_children(0);
    size_t myidx = hv.add_child( 0 ); //add child to root;
    my_hierarchical_idx = myidx;

    //REV: get root's dir... i.e. my parent...
    std::string parentdir = hv.vl[ 0 ].getvar( "__MY_DIR" ).get_s();

    mydir = parentdir + "/" + std::to_string( myidx ); //REV: whatever, to_string shouldn't cause a problem here, other than not padding zeroes.


    bool succ = make_directory( mydir );
    
    variable<std::string> var1( "__MY_DIR", mydir );
    hv.vl[myidx].addvar( var1 );
        
  }
  
  
  
  void add_pitem( const pitem& p )
  {
    pitems.push_back ( p );
    return;
  }
  
  
};

//REV: need a way to set that it is "done" too, i.e. go through and re-write back the files (?) to master, and check they're all there, and BAM.
//Furthermore, need to only give next pitem (next pset) when that is done.
//Anyway, need to rewrite this, or make a 3-rd party struct that keeps track of "done"-ness. Problem is if we check done-neess of previous but unneeded
//guys at local worker location, will fail because we just didnt copy files locally to TMP so be careful. Mm.
//Waste of time to look through ALL parampoints after gen 300k or something, so best to have a "start" point or something? That we periodically check.
//That makes the  most sense, not elegant though.


struct parampoint
{
  std::string mydir;

  std::vector< pset > psets;

  pitem get_next_pitem()
  {
    for(size_t p=0; p<psets.size(); ++p)
      {
	if( psets[p].checkfarmed() == false )
	  {
	    return psets[p].get_next_pitem();
	  }
      }
  }
  

  

  
  //Can do "check done" but better to just "get next pitem"

  bool checkfarmed()
  {
    for(size_t p=0; p<psets.size(); ++p)
      {
	if( psets[p].checkfarmed() == false )
	  {
	    return false;
	  }
      }
    return true;
  }
  
  bool done=false;
  bool checkdone()
  {
    if(done)
      {
	return done;
      }
    else
      {
	done = true;
	for(size_t p=0; p<psets.size(); ++p)
	  {
	    if( psets[p].checkdone() == false )
	      {
		done = false;
		return done;
	      }
	  }
      }
  }
  
  pitem get_next_pitem()
  {
    for(size_t p=0; p<psets.size(); ++p)
      {
	if( psets[p].checkdone() == false )
	  {
	    return psets[p].get_next_pitem();
	  }
      }

    fprintf(stderr, "REV: get_next_pitem: ERROR, we are trying to get next pitem despite PSET being ostensibly finished! Error!\n");
    exit(1);
  }

  //Makes the dir at the location "inside", but success and required files are still treated as if they are purely global names.
  parampoint( hierarchical_varlist& hv, std::string dirname )
  {
    mydir = dirname;
    variable<std::string> var1( "__MY_DIR", mydir );
    hv.vl[ my_hierarchical_idx ].addvar( var1 );

    bool succ = make_directory( mydir );
  }


  static const size_t my_hierarchical_idx = 0;
  //REV: this will always be the root node in the parampoint hierarchical varlist array...
  
};



//This should work. I need to repeat this, and push back the new guy in place of the old one. Repeatedly until all check_atomics are == 1
//not threadsafe heh.
//Wait, this will lead to a problem, in that we will only call this if it is not a SET_VAR.
//Wait, we also want to call this on SET??!?!!



//REV: this is absolutely nasty. I will return two things: one is the new (checked) SET_VAR with some guys reset. Other is list of new STMNTS


//Make a thing that takes the client::STMNT or LEAF_STMNT, and produces
//an executable thing (based on variables). Note, FUNCTNAMES cannot be
//variable I assume...? I could do that later.

//So, takes a STMNT, poops out a thing that actually searches for the function, binds the executable functional, and has args, and allows it to be "called", along with variable references etc. to extract it? In the end, functions all just take string lists. They're guaranteed to be first-level.
//Functs always return a string, and take a string vector argument?
//nargs is the

//REV: basically takes a "stmnt" passed by user and applies this to it.

//REV: Functions must have access to appropriate variable lists to do things.




//REV: OK so this is it. This is what we build. Note this is a SINGLE representation of a PSET (not a parampoint)
struct pset_functional_representation
{
  functlist myfl; //default constructed? Single static? No state.
  std::vector< functrep > frlist;

  size_t pset_width; //width i.e. num threads or whatever of this pthread.
  std::string myname;
  
  //Takes some registered functions struct (list?), and the (unrolled) stmnts thing.
  //    std::vector< client::STMNT >& stmnts ) //so, we take the STMNT list of this pset, and make what we need.
  pset_functional_representation( const client::PSET& p )
  {
    myname = ps.NAME;
    pset_width = std::stoul( p.NREP );
    for( size_t s=0; s<p.CONTENT.size(); ++s )
      {
	functrep tmpfr(p.CONTENT[s], myfl);
	frlist.push_back( tmpfr );
      }
    
    //frlist now has the recursive functional representations that we can call each of frlist[f].execute( vectofHIERCH, myHIEARCHindex)
  }


  //REV: TODO: This will do things like get actual worker locations? What about existing data/required data for each worker etc., stored/written to TMP?
  std::vector< worker_functional_representation > generate_workers( hierarchical_varlist& hv )
  {
    std::vector<  worker_functional_representation > wfrs;
  }
};


struct functional_representation
{
  std::vector< pset_functional_representation > pset_list;
  
  //Constructs the list of each parampoint. Great.
  functional_representation( const client::PARAMPOINT& pp )
  {
    for( size_t ps=0; ps<pp.psets.size(); ++ps)
      {
	pset_functional_representation pfr( pp.psets[ ps ] );
	pset_list.push_back( pfr );
      }
  }
};



struct executable_representation
{
  client::PARAMPOINT ppscript; //This is the list of PSET things, which are lists of STMNTs
  //REV: current problem is that we can't have statements outside of psets? I.e. we want global PARAMPOINT things to have an effect.
  //Let it be for now heh. THIS IS JUST THE SCRIPT SIDE. Need to go build the function side too.
  //To do that, the ppscript needs to have access to the VARIABLES themselves? No, not yet. I can build it without that.
  //But, I need to know what the functions do? I.e. I need POINTERS to the actual functions. Right. So I build that. And for stuff like
  //for loops etc., if they are nested in the script...no they won't exist. I.e. there's no online parsing of loop variables etc.

  //This is the functional representation of the script, specifically a list of the things to do.
  functional_representation fscript;
  
  executable_representation()
  {
    //Do nothing, will reconstruct anyway.
  }
  
  //build the exec rep using config file(s) passed by user.
  //For now, force it to be a single file...
  executable_representation( std::string script_filename )
  {
    //constructor. Constructs all the stuff, based on input script FILENAME?

    std::string input = get_file_contents( script_filename  );


    //construct the actual PARAMPOINT (config file representation?)
    ppscript = parse_psweep_script( input );

#if DEBUG > 1
    fprintf(stdout, "PARSED PARAMPOINT, got [%ld] PSETS\n", ppscript.psets.size());
    for(size_t a=0; a<ppscript.psets.size(); ++a)
      {
	fprintf(stdout, "PSET # [%ld]\n", a);
	enum_pset( ppscript.psets[a] );
      }
#endif

    //construct the executable representations based on that. I.e. construct for all PSETs inside it, the actual PSET things.
    fscript = functional_representation( ppscript );
    
  } //end constructor


  //FSCRIPT now contains the list (vector!) of pset functional representations, at construction.
  //Constructing a parampoint involves actually executing and generating the workers based on the passed param vals passed.
  //I.e. for each
  //  pset p in fscript.pset_list[p], do
  //   for each stmnt s in fscript.pset_list[p].frlist[s], do
  //    fscript.pset_list[p].frlist[s].execute(hiervarlist_vect, mypitemidx_corresponding_HVL)

  //Make certain set functs like getworkingdir(), etc.

  // To make this easier, I can literally generate the workers. Which have effects, such as filling REQUIRED, and SUCCESS file variables
  // (to actually check), as well as CMD variable. And other variables, and other side effects such as directories to generate, move commands to execute,
  //etc.

  //Builds and returns a parampoint. Which can then be executed in its own way.
  parampoint build_parampoint( hierarchical_varlist& hv, const std::string& dir )
  {
    //REV: this will not work lol, I need one of my own?!??!!
    parampoint mypp( hv, dir );
    
    //make each pset...
    for(size_t p=0; p<fscript.pset_list.size(); ++p)
      {

	//This does the actual execution? Nah, but we might set DIR? Ah
	//Remember I control the DIR format, I need to decide how to
	//organize the DIRS of working etc.
	//This will add the child, and then set the my_hierarch_idx to
	//the correct newly added one.
	pset mypset(hv);
		    
	//I will now have a PARALLEL list of the pitems. But those are each one generated by a running of the whole script, which is what the
	//pset_list is (stmnt list).
	//So, I need to somehow locally generate/modify the local hierarch varlist to set e.g. INDEX NUMBER etc. Do it manually here? Based on where
	//I iterate through each of NUM_PITEMS or something?
	//I need to set

	//pset_list[p] holds the pset_funct_rep, which has members like X.pset_width, which is # of parallel guys to do for this. Use that to set
	//a variable (i.e. which "index" number am I?)? Need a way to set (in the config) to e.g. set to FILE_X.dat etc., X is my #. And need a way
	//to specify REQUIRED files as e.g. PSET_N/PITEM_X/FILE.dat, and it will resolve those variables to their actual directories. I can handle
	//that within my hierarchical thing, I go up to the PSET, go up, move over one, etc.. Do I know which "child" number each child is? Ah, that's
	//the problem. If we only care about "global" numbering that's fine.
	//Either way, need a "get my index" type thing.


	//Add child to HV here, set myidx to that added guy.
	for( size_t w=0; w<fscript.pset_list[p].pset_width; ++w )
	  {
	    //PITEM CONSTRUCTOR WILL EXECUTE THE GUYS IN ORDER TO
	    //GENERATE CMD?
	    //Constructor does the modification of HV?!?!!! OK.
	    pitem mypitem( fscript.pset_list[p], w, hv );
	    mypset.add_pitem(mypitem);
	  }

	mypp.add_pset( mypset );
      }

    return mypp;

    //This will be the list of PSETS, and list of PITEM. And certain "special" variables will be set telling of required files, success files, directories
    //to run in, etc. I.e. at runtime, only CMD will run, all the other stuff is static.
    //Furthermore, "READ OUT" variables are specified by setting filenames that way. So, it will read those out and pass back to either main program or
    //whatever. Each PSET has possibility of returning? Nah, each PITEM even. But for fitness, it will return only from PPOINT!
    
    
    //What does this actually do? It doesn't return anything? It needs to actually execute everything and finally set the CMD variable. I.e. it modifies
    //all variables, etc. Can other things have side effects such as creating/removing directories, changing directories, etc.? Will the thing create
    //them automatically when it tries to go? What about stuff like setting /tmp?
    
    //Runs the function for the stored script and functional representation...
    //This will execute everything down to the CMD. It will also set all appropriate required variables etc.?
    //Ah, best idea is to have this generate lists of worker_functional_rep, which include e.g. SUCCESS and FAILURE. And then finally it will read
    //the CMD variable. Um, CALL it, with SET_CMD(blah). That is what will be executed later, using like "mydir" etc.

    //We need to know "which" thing to set it to. I.e. we need to "create" the individual guys, OK that's fine. We create them one at a time...and then
    //go execute them, checking success files of previous (no they are checked after), and required files of this time. And we make the guys to execute.
    //And we return the list of those. We need to make sure they're executed in order though, crap. *THAT* is what an executable representation is.
    //It will literally be farmed straight out to workers.

    //Ah, so we literally build our own set of worker representations in that order.
    
    //How about getting stuff out? We are setting some "specific" variables for this "sweep type", we need to tell it where those are. Or tell it to
    //read those into a special varlist? As results. I.e. this expects some specific variables to be set by this config file. To tell where the results
    //will come from/what those files will be named. Like "add results file". etc. They will always be in format of variables? And be read in that way?
    //name the files and read them in accordingly. Meh.
  }
};




//REV: 1 Jan 2016. I'm going through this linearly.
//1) We will have a farm funct that calls a parampoint generator to compute the results for a given parameter point. Passed as a single varlist ?
//2) This requires having the (specific) parampoint (variables) set properly. It also requires the SCRIPT for how to generate each
//   param point.
//3) This is decomposed into the individual PSETS () and PITEMS (). It checks required files etc. and resolves variables at that point.
//   We only execute the PITEMS at execution time
//4) We finally generate the CMD string (variable), which is executed at the end of the script (?)

//We construct a parampoint generator which has the set of named vars (the global ones), as well as the (list of) param point vars for each param point.
//Those are (constructed by the) made by the script. OK, start out with root which has the specific ones passed to me...


//Something will call generate in this. Specifically, the farm_funct. There is one of these made for the whole sweep full of the named variables etc.
//I need to specify the exec_rep (which is the list of executable statements for each PSET), which is generated from the script in turn.


//Two methods, one to get next pitem (and parampoint corresp parampoint num? Bc I need to copy it duh -_-)
//One to check if it's done. I.e. I need to know whether it is finished/successful.
//Another performs the actual farming out and sets to "being processed". Furthermore we mark which worker it is that is doing the processing?
//Have a list of currently processing. PARAMPOINT/PSET/PITEM. Linked list or something... deque.


//REV: this is the structure that operates almost entirely on param point space
//It also knows what needs to be done to detect convergence etc.?
//User passes a function pointer or something? Or there are several "algorithms" that are selected.
struct search_function
{
  
};

struct parampoint_generator
{
  //This needs to have the ability to take current list and generate new ones?
  
  std::vector< hierarchical_varlist > named_vars;

  std::vector< hierarchical_varlist > parampoint_vars;

  std::vector< parampoint > parampoints;

  std::string basedir="./";

  size_t first_active=0; //this is first in array of parampoints that is active i.e. not finished.
  
  size_t paramptidx=0;

  //It's a single executable representation...
  executable_representation exec_rep; //representation of the "script" to run to generate psets (param point) etc.

  //Construct it by specifying global vars or something?

  parampoint_generator()
  {
    //do nothing, dummy constructor
  }
  
  parampoint_generator( const std::string& scriptfname, const std::string& bdir )
  {
    exec_rep = exec_rep( scriptfname );
    
    basedir = bdir;
    make_directory( basedir ); //better already exist...
  }

  
  //takes an argument which is just a (new?) variable list of the parameters to test, and does stuff. Note some (other fixed parameters) might be global.
  //This ONLY generates, it is totally independent to sending stuff to be executed.

  //I would like to totally dissociate the farming out (of work) from the search function.
  //To make new param points, we do that by calling for new points. No, something is on top of that I guess. I.e. something calls the VL thing.
  //And then farms out the work (inside this structure?) until it is done
  //I.e. control passes to that to farm out that "set" of paramthings at a time? May be a generation, may not be?
  //We pass a set of varlists to execute, and it does everything for me.
  
  size_t generate( const varlist& vl )
  {
    //Takes the varlist...
    hierarchical_varlist hv( vl );
    
    parampoint_vars.push_back( hv );
    
    std::string dirname = basedir + "/" + std::to_string( parampoints.size() );
    parampoint retp = exec_rep.build_parampoint( hv, dirname );
    
    parampoints.push_back ( retp );

    return (parampoints.size() - 1);
  }


  
}; //end struct parampoint_gen


//If I set up a "farmer" device, it will contain a parampoint_gen,
//but I'll need internal access to which ones are active etc.
//And a way to signal when a specific worker is finished.
//Need to know parampoint # and worker number in it. It will "wait" to
//check it, write back the files (if neccessary), and mark it done (check
//files locally too). Make MD5 or something?

//I can spin through all of them, checking. Or just wait to receive
//ANY message, I think that is possible.


//What is executed in main thread? There is some main search algo, that we call, i.e. we construct a search funct. And we simply iterate
//it until it is "done" But the search function also constructs (by itself) a parampoint_generator for handling vls at a time. The user
//passes that separately. I.e. he makes a parampoint_generator by himself




//Parampoint generator -- generates an actual parampoint (psets etc.) based on originally passed SCRIPT, and also varlist containing PARAMPOINT
//to evaluate.
//Only generates when called. Keeps track of parampoints as well, i.e. history. Keeps track of which have been "farmed", which are "ready", and
//which are "done". Note each parampoint struct keeps track of this in addition to other information. That is bad, should separate that out? Meh.

//Furthermore, there is a search function that gets outputs from finished parampoints, generates new ones, etc.. I.e. that is the controlling runner.
//It gets a pointer to parampoint_gen and passes guys to it to get out things to run. Finally, when it is finished, it somehow gets out the
//results and does something with them.

//Is the search function at all related to how I actually farm out the guys? No, has to do with workers. Set a "max farmed at a time" type variable,
//so it writes that many parampoints to file as it goes. Much easier that way no obvious "generations" or something.

//Every generation could be a file (?). Easier to have one large file... in which case we need to handle errorful writing? We can correct it...erase
//most recent generation? Or do checking of success.

//What does the SEARCH function do? It has state I assume, may require responses before continuing, or not?
//It generates a bunch of guys at the same time, computes them, gets results, writes to disk, etc., until we decide it's done.

//OK, this is "at a time" value that was selected. However there may be "stops" e.g. if not enough to get a generation. So we don't know what
//it uses to generate the next set of guys. But we know it must be passed a VL as a result. It has its own unknown method for generating.
//Base class, and make it a higher level class. I really don't want to use a pointer or some shit like that?

//Takes generic hierarch varlist that handles things?

struct search_funct
{
  search_funct( hierarchical_varlist& hv )
  {
    
  }


  std::vector< varlist > gen_next_parampoints()
  {
    //All different search functs must do this. It calls this, and it gets the next N param points, which are then farmed off.
    //Note this includes "getting the output". May contain current state? Note, every pitem returns OUTPUT.
  }
  
  //So any derived classes might override this.
  //It generates (possibly from previous or from nothing),
  //executes, and then generates based on results? Generates 2x? Nah.

  
};



//Passes these off to the "worker distributor" and does the farming etc.
struct master
{
  //Easiet way is to carry around a "full" one, and only push/pass a few to it as we go. I guess.
  //Or, specify ones to execute?

  farmer thefarmer; //keeps track of the system, i.e. where GPUs, etc. are located, IP to connect to build MPI thing, etc.
  
  parampoint_generator maingen;
  
  master( scriptfname, bdir, )
  {
    
  }
  
  
  std::vector< varlist > process_param_points( const std::vector< varlist >& pp_toproc )
  {

    //what calls this? We call this each time? NO THIS IS PART OF THE "PROCESS" FARMER-OUTER.
    //Assume we do everything AFTER having passed the varlists etc. to the target? We just pass parampoints I guess?
    //What about vars etc.? At any rate, once it gets the parampoints etc., it goes off. So we need to pass the whole struct including
    //the lists it has. Pass both the things I guess haha.
    
  }

};

struct farmer
{
  //Pass back/copy back the whole thing? Ugly...vicious!
  //But we're writing out to HDF5 or something somewhere along the way,...
  //I.e. as its going its writing it out? Only after "completion" of a given parampoint. But dirs are already made when farmed out? Only made as
  //sent to workers?

  //Make user bundle all his own functions (in the same language?) or possibly glue it all together, but with main as C++?. Or give it as a lib...heh.).
  //If that's the case, no need to specify all the files and shit though ;). But if each one is running as "system" that's fine. But if user has spawned
  //so many guys, hm. We can't run multiple kernels on same guy.

  //For example, if we exit by accident partway through a whole chunk of "SWEEP" guys, we'll still have the guys that were done. I.e. we want to be able to re-start
  //part way through? But, that is difficult if they are all done at once. Need a way to save "processing" state if its a really big guy. Whatever, figure it out later.
  void comp_pp_list( parampoint_generator& pg, std::vector<varlist>& newlist )
  {
    //Only compute newlist. Make a "completed" thing for each one...
    std::deque< completion_struct > progress;
    for( size_t n=0; n<newlist.size(); ++n )
      {
	//make the parampoint.

	size_t idx = pg.generate( newlist[n] );

	//This constructs a thing matching the specific parampoint shape. 
	completion_struct cs( pg, idx );

	progress.push_back( cs );
      }

    //I will delete them as they are completed from deque?
    //Or I need them for re-writing info back? To pg.
    //I need to have some list of "workers" or something to look through to
    //check everything.


    //Have a list of workers, and some way to initialize them all? They are all initialized themselves somehow? I need to somehow get info about them?
    //Allow other guys to register online with me? Or have it all start at beginning with MPI?
    //Keep track of WHICH worker I sent WHICH info to?
    //Yea, otherwise I need to look for which completion struct has this worker as the farmed worker...
    //Just get a pointer to it easier..ugh.

    std::vector<parampoint_coord> currentworking;
    
    size_t wait_for_worker( std::deque< completion_struct >& prog )
    {
      //Note iprobe is NON BLOCKING, probe is blocking?
      //use MPI...easier?
      //http://mpitutorial.com/tutorials/dynamic-receiving-with-mpi-probe-and-mpi-status/
      MPI_Status s;
      MPI_probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
      int mesgsize;
      int sourcerank = s.MPI_SOURCE;
      //This is assuming its int?
      MPI_Get_count(&s, MPI_CHAR, &mesgsize);
      char mesg_buf[mesgsize];
      
      MPI_Recv(mesg_buf, mesgsize, MPI_INT, 0, 0,
	       MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if( strcmp(mesg_buf, "DONE") == 0 )
	{
	  //This is from a finished guy. Just set it to waiting and loop back.
	  //We need to "read in" and handle all the stuff sent by the worker.
	  //This includes copying over the filesystem,
	  //This may include LOTS of data
	  transfer_data td = get_data_from_worker( sourcerank );
	  //Handle it, i.e. write to filesystem etc.
	  //This writes all "files" to "name" and "file" in memory. We need to know corresponding place to write here as well though.
	  //And, we need to appropriately modify parampoint that finished, etc., with the results?
	  
	}
      else if( strcmp(mesg_buf, "WAITING") == 0 )
	{
	  //this is first time, no need to get result from target. 
	}
      else
	{
	  fprintf(stderr, "Huh, returned mesg from rank [%d] containing [%s] is not expected content\n", sourcerank, mesg_buf );
	  exit(1);
	}

      //OK, got it, now I need to get more JUST FROM THAT TARGET, until it is done. E.g. finish up stuff. I get some "chunk" of results, which is what, a structure?
      //Need to use serialization after all ugh...
      
      //Literally spins waiting for a signal from any of the workers.
      //Two conditions
      //1) it has finished some work I gave it and now everything must be copied back and checked.
      //2) it didn't have any work, so just farm out (i.e. first spin through).
    }

    bool checkalldone( std::deque< completion_struct > & prog )
    {
      
    }

    bool check_work_avail( std::deque< completion_struct >& prog  )
    {
    }

    void farmwork( std::deque< completion_struct> & prog, const size_t& workernum )
    {
      
    }
    
    //this will contain somem implementation of a communicator method? 
    
    while( false == checkalldone( progress ) )
      {
	//block, waiting for a worker to tell that it is ready (or to finish?)
	
	
	//wait for a worker to be open (or to get it back)
	//This will both "handle" a successful return data (if that is what happened)
	//and/or it will also.
	//This will impl with MPI, or maybe HADOOP etc.
	//Why do we need to know what worker it is? We know at least ONE is open.
	//Does only one worker (?). We could do this on multiple threads if
	//we really wanted to be cool ;)
	//This copies all stuff back to this side, but to where? To PG? I guess.
	//Copying back just the varlist, or all the filesystem that was possibly
	//created at /TMP???
	//It will update progress. Will it also do something fancy, writing back
	//to correct location in filesystem? It must ;) We must have our "local"
	//guy here, OK. So, it writes to PG locations the corresponding stuff
	//from the other side fS? How does it know? We had to have built
	//a correspondence on this side/that side somewhere. In the PG?
	size_t openw = wait_for_worker( progress, pg );
	
	//wait for work to be available (or, check if it is?)
	//If not, loop back to top.

	
	
	//Checks if work is available
	bool iswork = check_work_avail( progress );

	if( iswork )
	  {
	    farmwork( progress, openw );
	  }
	//Else, loop back to top, to check and wait for a worker to be open...

      } //end while we're not all done
    
    
    
    //Passes as memory, so much easier... I.e. it modifies it in line...
    //OK, and I do everything. This is all on the master rank.
    //Keeping stuff here allows me to not mess around with internals of parampoint_generator other than to get the guys from it?
    
    //Do all "in processing", "output to archive", "done" checking, and "send to workers" here.
  }


  //local struct that keeps track of which:
  //PARAMPOINT, PSET, and PITEM have been farmed out, to where/which worker (will depend on implementation?), which are done, etc.

  struct parampoint_coord
  {
    bool working;
    size_t parampointn;
    size_t psetn;
    size_t pitemn;

    parampoint_coord( const size_t& pp, const size_t& ps, const size_t& pi )
    : parampointn(pp), psetn(ps), pitemn(pi), working(true)
    {
      //REV: nothing to do.
    }
  };
  
  //keep an index for each of the PARAMPOINTS in pg, that tells stuff for each.
  struct completion_struct
  {
    std::deque<> active_pps; //contains um, list of pps, as well as each "index" of each, i.e. where it is in the pg struct?
    //Fuck, so much better to just include this in the parampoint_gen thing. But then we don't know what format workers are etc. So better to
    //separate it out.
    //But we "know" what format of parampoint etc is, i.e. list of psets, and each pset is a list of pitems. OK.

    struct pprep
    {
      size_t pset_idx;
      size_t pitem_idx;
      //a pointer or marker?
    };
    
    struct parampoint_rep
    {

      bool checkdone()
      {
	if(done)
	  {
	    return done;
	  }
	else
	  {
	    done=true;
	    for(size_t p=0; p<psets.size(); ++p)
	      {
		if( psets[p].checkdone() == false )
		  {
		    done=false;
		    return done;
		  }
	      }
	  }
	return done;
      }

      
      
      size_t myidx;
      size_t current_pset=0;
      bool done=false;
      std::vector< pset_rep > psets;
    };
    
    struct pset_rep
    {
      size_t current_pitem=0;
      bool done=false;
      std::vector< pitem_rep > pitems;

      bool checkdone()
      {
	if(done)
	  {
	    return done;
	  }
	else
	  {
	    done=true;
	    for(size_t p=0; p<pitems.size(); ++p )
	      {
		if( pitems[p].checkdone() == false )
		  {
		    done=false;
		    return done;
		  }
	      }
	  }
	return done;
      }
    };
    
    struct pitem_rep
    {
      bool checkdone()
      {
	return done; //actually check it?
      }
      
      bool farmed=false;
      bool done=false;
      size_t farmed_worker=0;
    };
    
  };
  
  
};
















//REV: HOW TO DO THIS.


//User builds the search function (specifies params, which function to use etc. In some main proram).
//User also specifies the method for building the param points by giving the script. Specifies #workers etc.
//The search function generates varlists for parampoints, and gets results back. That is all it cares about.
//It knows nothing else.

//Separate than that, we have something that executes parameter points. It goes through a list of parameter points to execute, and does them.
//The list is a LOCAL list (not the full list) of only "this turns" param points.
//So, we build a mini param point generator, and pass that as a chunk, get the results all back.
//Inside this mini list, we will be distributing to workers.

//I guess this is the point at which we write out to HDF5 or at least write out regularly, because e.g. if it is a SWEEP then there will be no
//"generations", and thus no stopping points (because it might be inefficient).

//This works best. Then, we might have a global list of all the param points after.





struct master
{
  //REV: this has a specific algorithm in it for generating the param points?
  parampoint_generator pg;
  
  
  bool spin=true;
  void run()
  {
    while( spin )
      {
	//1 Attempt to generate new param points.
	
	//2 Spin until they're all done? Need a place to cut/push HDF5.
	
	//Best to do them as they are done? In "generations"? Nah...
	//Just one PP at a time.

	//This will be determined by the parampoint generation algorithm. It may generate them in a chunk though. E.g. as generations.
	
      }
  }
  
  
}; //end struct master









    //REV: we now have the actual parampoint, we need to/want to execute it now.
    //Also, parampoint_vars has the hierarchical varlist pushed back.
    //We need to actually EXECUTE it in order, doing the CHECKS and running CMD with SYSTEM, getting RESULTS, etc.
    //Note, as of now this just generates parampoints, but it also makes the required dirs etc. We need to literally
    //have the "signal" to each to execute the CMD. But, we need to "get" the cmd. Note, will each worker independently handle itself? Yea,
    //it can all do it itself. We can pass the pitem to the worker, and it will handle everything ;) Otherwise, local /tmp won't work or something.
    //So, serialize it etc. So, the local worker needs to do execution when passed the CMD (in fact the thing itself. FUCK PARAMPOINT CREATION MUST
    //BE DONE BY THE WORKER LOCALLY, or file stuff might be messed up. I.e. construction must be done there. That is not good. I.e. the actuall).

    //REV: why do this here? If we will write it out of course. Problem is of course what? Ah, PSET must be made locally. But we can make/do everything
    //for workers, then copy it to the correct location. That is how it SHOULD work. We ASSUME there is a network drive (lustre etc.) of some kind.
    //Why am I using this and not HADOOP like thing? At any rate, workers give the thing the targets to write to locally (i.e. place to execute from?)
    //I assume we move everything to the RAMDISK. And then copy it only after it is done. I guess that works. Same worker is not guaranteed to do
    //every pset of the guy we need, so in between we need to copy back to the shared file system (and/or have some way to communicate to targets the
    //stuff they need). If we communicate via file system it works with LUSTRE, but it will not work if the file-systems are totally separate.
    //If the file-systems are separate, we have a problem. So it's best to make a shared file-system. But, we could do it over networking too. In other
    //words, move everything via network from all the workers to the "master's" system after it is finished. Question is whether this is worth it or
    //should we give "afinity" to a single box for all pitems? Of course, but meh. How long will copying of results take? Could take a while. But, it
    //is on DISK, so we need to READ FROM DISK, and SEND TO MASTER. Figure this out later...for now we are using shared file system as a hack.
    //To send it back over MPI, it is a bit of a pain in the ass. Because the "master" needs to receive it. Can we have the "master" spinning? Ugh.
    //Should we only send "output" files, or EVERYTHING the user's program completes? Note, of course SUCCESS files are needed. And they are same as
    //next's REQUIRED files.

    //Option --USE FS (accessible from all workers), or

    //Best option: Workers have a "scratch" space, they "know" to automatically copy everything to /tmp (or not), do their work, then copy back.
    //Furthermore, every worker (when created) has all the files he needs created. Note, user specifies "required" files, he can specify "data: X" to
    //tell to get it directly from DATA location, rather than typical copied location. That way it is success file.
    
    //How about normal files? They "require" from worker, but heck they'll all be copied. Make everything with a "worker-base" which can be modified.
    //In other words, um, all file paths are modified to be from worker base. At worker spin-off time I guess. I.e. the "overall" guy is already created,
    //then everything is copied and executed by worker.

    //PROBLEM:
    //In CMD, we need to construct at WORKER time!!!!! Which is the real problem, because we may need to specify to USER PROGRAM via CMD that
    // it should use FILE X FILE Y etc., which are IN /TMP or something, not the original locations in shared network drive.
    //So, although we get relative file locations from user, and it creates them in shared space, at actual execution time (CMD construction time?)
    //we modify the corresponding file paths to refer relatively to worker /tmp base.

    //How to do this? When a WORKER gets a PSET, it gets the raw unadultered one? All file references (required files) are COPIED (OK...fine?)
    //and all data files are already existing (likewise?). At /tmp locally. Problem is, we need to modify the REFERENCES in the script (specifically
    //in CMD) so that they refer to proper TMP locations. And this needs to be done BEFORE cmd is generated, i.e. before EXE happens.
    //Creation of dirs output of files might take time, so why not just delegate it to workers themselves. I.e. send the parampoint to the worker,
    //and have it do the required creation? Nah, pain in the butt. But, creation of the PITEM is good. Problem is e.g. there might be "required
    //input" from previous PSETS, thus we can't just copy the worker dir. One major problem is that multiple previous PSET PITEM might have overlapping
    //filenames!!!! E.g. if they are all named equivalently "output". Maintaining a copy of everything on each machine is out of the question I assume.
    //However, we can maintain local FILE SYSTEM reference locations? So, recall things like PREVIOUS_PSET( outputfile ) must appropriately resolve
    //AT RUNTIME to point to local TMP locations, where they were copied or something (i.e. dirs must be set). So, after all, we will create a whole
    //DUMMY directory sturcture of each PARAMPOINT at each WORKER, but will only FILL as necessary. How do we coalesce? Well, we "writeback" only the
    //CURRENT (this PITEM) directory to the global. I like that. So, all directory references must be constructed/generated online with respect
    //to PARAMPOINT-HEAD (or "DATAHEAD"), which in case of /TMP system will be temporarily reset to /TMP absolute file location BEFORE generation of
    //CMD (i.e. of that PITEM). However, it must be done globally first, so that all PITEMS write out their appropriate files in the first place? No,
    //that is not necessary. Just go one by one, creating and running. When we pass to a worker, we already are checking REQUIRED/SUCCESS files,
    //from the "already run" pitem creation, and we copy those accordingly to the corresponding /TMP file locations (automatically, if they exist).
    //Then, we RECONSTRUCT the PITEM at the target location (we don't bother creating the dir right??? We just need to change file locations in
    //variables/strings. That is the problem. Best to have them all "constructed" from a source at runtime to generate the CMD. We don't generate
    //CMD until the very end, after we may have changed "base" to appropriately reflect everything. I like that idea.

    //Still minor problem of how to copy/create all the "old" guys from previous PSETS at the /TMP location...
    //Also recall that we have some "minor" problems in that some functions can have other side effects, such as creation of things, running
    //programs etc.? So, require user to have a "setup" that specifies required/blah files before a "run" cmd? I.e. creation of the CMD variable.
    //Nono, all stuff to set CMD is set...shit. This doesn't work. User's thing will actually create teh CMD anyway ugh. So, yea, we need to
    //re-run it again, but this time, with all success files etc. locally, so that CMD resolves correctly.
    //If the script has side effects or something, that is a minor problem, as we will be running it twice...
    //Shit, it will be adding variables etc., that is the problem!
    
    //So, we will just execute the "set_CMD" command one more time at the very end, but now the passed varlist will have "basedir" set to something else.
    //Note everything is strings or shit, so it has no idea that the STRINGS are FILE PATHS. So that is just nasty, because we don't know when user
    //may set a string var to a file path and then use it later without calling the correct "function" to get it in a locally valid manner.
    //Let user make all file references in some way that is safe, i.e. give a function that pads the string at beginning with the base dir.
    //OK, and then hope user uses that if he knows what he is doing ;) Like PREFIX_BASE_DIR( blah ). of this pitem, or a previous psets Nth pitem, or
    //whatever. Note that an array of variables needs to be done in that way too..? For reading arrays of previous guys, ugh. So, we will SKIP all
    //SET_VARS? No, that won't work. At the end, our only option is to run the user script, get the REQUIRED/SUCCESS variables copied. Then,
    //we will NOT automatically parse everything to fix the paths... but we DO need to re-run everything to re-generate from scratch.
    //AH WE CAN DO THIS JUST CLEAR OUT THE HIER VARLIST AND START FROM SCRATCH. But now with everything set to /TMP. OKOKOKOKKOKOKOK. GO.
    //This assumes EVERYTHING IN VARLIST WAS CREATED BY THIS SCRIPT, i.e. there is nothing else in the varlist.
    //But we might be getting varlist stuff from elsewhere (e.g. named etc.?). Nah, disallow that.
    
    
    //So, todo is to make sure that user specifies files all as happening globally. E.g. required files and success files etc. Those are all COPIED
    //to a given location, and then file paths are set to those new locations I assume. Problem is of course before CMD generation, which may reference
    //those. Even if it's global, we still need to SEND to worker on different computer. Have that all happen at start (copying data files).
    //But that's a problem as we have to generate them? Nah...but its "required" for a given guy. I.e. specify "data" files. Don't recopy every time?
    //Assume that um, individual workers will have blah? Ah, for each worker. Based on script? Ugh. Make user specify everything in there?

    //At any rate, send it all over a port? Or MPI setting? I guess I can serialize it all as binary and send over MPI ports, then write to file
    //just like that. Seems a waste to send over MPI though..? But is there any other way? SSH or SCP or some shit? Either way it will go over network,
    //and be written/read to file. Specify the "data folder", and just leave it there or something? "Generate" name lists?

    //So, this is part of the "actually executing". First, I run it, and do all the copying etc. to the worker who has "hit" it. Passing the parampoint.
    //Build the structure at the worker's /TMP location. The worker has its "base directory" in some way, just like other guy. But all guys are copied
    //with relative path to TMP, e.g. in data files etc. If there are name clashes, we have a problem? E.g. multiple global dirs with sub-dirs with
    //overlapping names etc.? That is a pain in the ass. If there is a clash, I will re-name it. But user program might not know I changed it, it might
    //assume a certain filename? E.g. if I pass a "data dir" but not individual data file names or something? Hopefully user doesn't do that. Don't do it
    //unless you will use it policy! Forget it for now, but it can be resolved heh.

    //So, at initial position, when calling, we send the varlist along with it so it knows. But, on THIS side, we do the "sends" of all the guys to the
    //selected worker. Who is "waiting" for it. Gets the stuff, writes it.

    //At the same time I have changed the names of all the files to be different/local. I.e. for each file I have send/copied (based on var), I have
    //changed the name to some arbitrary local one at the target location. They may start in some diverse locations though... so I need a way to
    //copy them? Just name them arbitrary things? Assume user program does not expect some named file or else. Meh, depending on loc? We can give
    //it new local name too or something? For each? But then we must pass it to CMD.

    //Offer function that has underlying impl. It may be MPI, etc. We don't know. Hadoop? OK, we can do that later (DFS?)
    //Workers sit and wait.
    
    //Hadoop?
    



//We have the parampoint_gen constructed. We need to build (another?) one of these locally, in other words this is passed in memory (ugh).
//Should only pass the necessary things....no need to pass previous param points etc.? Passing contents of vectors etc. pain the but wow...
//Anyway, so I pass the constructor thing along with the filenames etc. So, first all the required files are sent with their new names and locations,
//and this side worker writes them to TMP or whatever.
//Note settings might be different per workstation? Ugh. Infiniband??? Ethernet speed times two at most... But disk write speed will be holdup.
//How long will it take to transfer what, 10 kb of data? At 10 Mbps that is already um, 10 ms, but 10 mbs, so 1/10 of that, i.e. 1 ms. Assuming we have
//6 K80s, with 2 each in them wow. That is 12 GPUs. So, with 12 GPU, we can be amazing. We could even run multiple on each (if we can). If I use same
//spawning worker, I could theoretically run multiple kernels on same GPU. But only if it is actually only executed from the same GUEST program!!!!
//I.e. the C++ simulation. So, that has to be executed from in the same thread even (!!!!!??). I.e. in concurrent. Ugh.

//Note, host program has to access GPUs automatically. Meh. Host # etc., they need to communicate which GPU is open! Pain in the butt I guess. And
//data about the GPUs. So I can get CUDA info into main program and use it to feed them stuff. Based on estimated size etc? But I don't know about that?
//Can I Know which are "open" and which not from CUDA probing?

//Either way I need to send it to the workers. So I have a worker, it signals its ready. I select it, I send it. The other side is waiting to receive
//some format?
//Just send as bytes I guess. Or ints. Or whatever. Must be bytes. Source and dest machine must match up I guess? Sending INT etc. Must send as chunks
//because file size might be very large (larger than INT_MAX). Make sure file write is not blocking? Have 2 threads or something...?
//

//Worker spins and executes only when got stuff. Same as other, send "ready" to master. Just as before, but now I send so much more. Send "new" filename

//dynamically load a function (model), pass data that way?

//User needs to include the header file of his lib to have it compute properly.

//I.e. force user to have his header etc. included, and then compile with .so included? No, can be after.
//Or force user to compile with???

//All nasty, still we're hitting LDAP too much or something. Is there a way to get current process to "switch" its identity? I.e. same thread runs,
//but we "override it" multiple times? Or we could spawn new one each time or something... We'd like to load everything at start time.

//I just realized I may be thinking of this backwards. Another highly likely way to do this is to create as a LIB, the sweep program, and then include
//that (as C++ include or shared lib) into my main C++ model thing. And then, each "rank" is just one of my main model things running.
//So, I include it, and then I in each thread, start the libs worker thing, which spins and waits? And when it gets it I simply call my own model code
//with the stuff from the main guy. Problem is that e.g. user must generate his own main function? Nah, just pass the function signature or something?
//At any rate, we are passing everything back as memory, which solves a big problem. This may be best/necessary for cuda implementation. Note that each
//worker can then execute SYSTEM call for non-C++ guys and communicate with files in that way. That is the best idea? It keeps C++ main while allowing
//other guys to happen. Problem is that user must modify his own code to use the library, and write the script I guess. Specify SYSTEM commands versus
//some other way of scripting things without modifying the user program main stuff. E.g. constructing CMD for other guys. For direct C++ guys, we need
//to specify what memory to pass to it? We know some things like variables, so we can just pass those directly. Force user to use the variables, but
//that's fine. We can pass them by name haha.


//At any rate, need to figure out how to communicate required files or whatever to target, as correct variable names (filenames)

