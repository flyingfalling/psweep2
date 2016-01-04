//TODO: make special functs to add file to REQUIRED/SUCCESS/etc.
//TODO: Make it so that we can also pass along/check "named" varlist
//TODO: make special functs to access previous psets/pitems (directories) so we can do that.
//TODO: Figure out how to organize directories/access them? Workers/etc.

//Questions: how to deal with variables over different PSETS in PARAMPOINT
//Questions: how to build CMD better? Add option type thing? Make CMD a str var array and have it do the stuff ;). And at the end it adds with seps
//           or something

//TODO: special variables to generate guys from array with other string, with certain sep. E.g. to access "all" the other worker guys. Need a way to
//      access programmatically how many PARALLEL guys there are? Yappa over PSET we want to have many. Have a special function to access #P or something.


#pragma once

#include <functrep.h>

//REV: this is a parampoint. It has a command thing that is what to run
//each time point. Note, we only actually want one of those representations,
//that is run each time I guess, applied to the specific parampoint.

//The parampoints need to have their variable lists or something. I'd really prefer to not have a global var list. But, we can always just pass it
//as an argument in functions.

//REV: this is a worker item. I.e. a single parameterized worker thing.
struct pitem
{
  std::string mycmd; //the main thing, this is the cmd I am running via system()?

  std::vector< std::string > required_files;
  std::vector< std::string > success_files;

  std::vector< std::string > output_files;
  
  pitem( pset_functional_representation& pfr, const size_t idx,  hierarchical_varlist& hv)
  {
    //Need to add to the most recent pset a child...
    std::vector<size_t> rootchildren = hv.get_children( 0 );
    size_t npsets = rootchildren.size();
    size_t myidx = hv.add_child( rootchildren[npsets-1] ); //add child to the last one.
    my_hierarchical_idx = myidx;

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

    required_files = hv.get_array_var( "REQUIRED_FILES", my_hierarchical_idx );
    success_files = hv.get_array_var( "SUCCESS_FILES", my_hierarchical_idx );
    mycmd = hv.get_val_var( "CMD", my_hierarchical_idx );

    output_files = hv.get_val_var( "OUTPUT_FILES", my_hierarchical_idx );

        
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
  
  std::string get_mydir()
  {
    //Return variable from my varlist? Or store a local (relative) path?
  }

  bool checkready()
  {
    //checks if all required files are present?? etc.
  }
  
  bool checkdone()
  {
    //checks if I'm done. Specifically by seeing if all required guys are
    //finished. I need multiple "variable lists" of guys, some that
    //set required guys for starting, some that check required files for
    //ending. Don't check file contents though.
  }

  varlist get_output()
  {
    varlist retvarlist;
    for(size_t f=0; f<output_files.size(); ++f)
      {
	retvarlist.inputfromfile( output_files[f] );
      }

    return retvarlist;
  }
  
  size_t my_hierarchical_idx; //index in my parampoint hierarchical varlist array of my leaf node.
};


//REV:
struct pset
{
  std::string get_mydir()
  {
  }

  bool checkdone()
  {
    bool done=true;
    //checks if this pset is done. Specifically, by checking if all (remaining) pitems are done! If not, wait until they return.
    for(size_t pi=0; pi<pitems.size(); ++pi)
      {
	if( pitems[pi].checkdone() == false )
	  {
	    done=false;
	  }
      }
    return done;
  }


  pset(hierarchical_varlist& hv)
  {
    //std::vector<size_t> rootchildren = hv.get_children(0);
    size_t myidx = hv.add_child( 0 ); //add child to root;
    my_hierarchical_idx = myidx;
  }
  
  std::vector< pitem > pitems;

  void add_pitem( const pitem& p )
  {
    pitems.push_back ( p );
    return;
  }
  
  size_t my_hierarchical_idx; //index in my parampoint hierarchical varlist array of my leaf (or middle) node.
};

struct parampoint
{
  std::string get_mydir()
  {
  }

  std::vector< pset > psets;
  
  //REV: this will always be the root node in the parampoint hierarchical varlist array...
  
};



struct worker_functional_representation
{

  std::vector< std::string > required_files;
  std::vector< std::string > success_files;

  //check the required/success files? Note, per guy, they might be relative? Hm, no, they will be constructed by pset_functional_rep already all
  //variables set.
  
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


  //REV: TODO
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
  parampoint build_parampoint( hierarchical_varlist& hv )
  {
    parampoint mypp( hv );
    
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
struct parampoint_generator
{
  
  std::vector< hierarchical_varlist > named_vars;

  std::vector< hierarchical_varlist > parampoint_vars;

  //It's a single executable representation...
  executable_representation exec_rep; //representation of the "script" to run to generate psets (param point) etc.

  //Construct it by specifying global vars or something?
  parampoint_generator(  )
  {
    
  }
  
  //takes an argument which is just a (new?) variable list of the parameters to test, and does stuff. Note some (other fixed parameters) might be global.
  void generate( const varlist& vl )
  {
    //Takes the varlist...
    hierarchical_varlist hv( vl );

    //This pushes back to parampoint_vars the hv. Which will build out the PSETS and the PITEMS beneath it.
    parampoint_vars.push_back( hv );
    
    //Make the children, we need to go through the executable representation
    //This will build the CMD strings down to the worker level. Can the things have side effects? Sure, they can. We will run EXECUTE on each.
    //Wait, this can't work, unless I have access to global/named and previous parampoint vars? Nah, those are not needed...
    parampoint retp = exec_rep.build_parampoint( hv );
  }
}
