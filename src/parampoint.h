



//REV: this is a parampoint. It has a command thing that is what to run
//each time point. Note, we only actually want one of those representations,
//that is run each time I guess, applied to the specific parampoint.

struct pitem
{

  std::string mycmd; //the main thing, this is the cmd I am running via system()?
  
  
  std::string get_mydir()
  {
    //Return variable from my varlist? Or store a local (relative) path?
  }

  bool checkdone()
  {
    //checks if I'm done. Specifically by seeing if all required guys are
    //finished. I need multiple "variable lists" of guys, some that
    //set required guys for starting, some that check required files for
    //ending. Don't check file contents though.
  }
  
  size_t my_hierarchical_idx; //index in my parampoint hierarchical varlist array of my leaf node.
};

struct pset
{
  std::string get_mydir()
  {
  }

  bool checkdone()
  {
    //checks if this pset is done. Specifically, by checking if all (remaining) pitems are done! If not, wait until they return.
  }
  
  std::vector< pitem > pitems;
  
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

struct registered_functs
{
  //(user-defined) or pre-coded Registered functs, and a way to search them by string, and argument size/number etc.
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




typedef (std::function< std::string(const client::STMNT&, std::vector< hierarchical_varlist >&, const std::vector< size_t >&) >) functtype;



//Make a macro. Can I expand it from template.
//E.g.

std::string SETVAR( const client::STMNT& stm, std::vector<hierarchical_varlist>& hv, const std::vector<size_t>& hv_idxs )
{
  //stm.TAG should be setvar
  //stm.ARGS[0] should be the VAR to write to (i.e. it's a LEAF_STMNT, and stm.ARGS[0].TAG is the variable to look up/write to. In lowest level I guess.
  //stm.ARGS[1] defines the value to write. It may be an array, and it may be a variable. If it is an array, how can we tell? Well that type of function
  //must somehow be a "variable" function (return an array). Is that possible? Do we know? Make two, SETARRAY and SETVAR. Fine...fuck.
}

std::string SETARRAY( const client::STMNT& stm, std::vector<hierarchical_varlist>& hv, const std::vector<size_t>& hv_idxs )
{
  //stm.TAG should be setvar
  //stm.ARGS[0] should be the VAR to write to (i.e. it's a LEAF_STMNT, and stm.ARGS[0].TAG is the variable to look up/write to. In lowest level I guess.
  //stm.ARGS[1] defines the value to write. It may be an array, and it may be a variable. If it is an array, how can we tell? Well that type of function
  //must somehow be a "variable" function (return an array). Is that possible? Do we know? Make two, SETARRAY and SETVAR. Fine...fuck.

  //The function called must return a fucking array, not a string? The nested function. Fuck, but I thought all functions always returned std::string?
  //Some return std::vector<std::string> I guess. That leads to a problem. We should make a tagged data type. In fact I do. All functions return fucking
  //VARIABLES. And variables are a tagged data structure that tells if it's an array or blah. Problem is we only use the array part in very specific
  //circumstances. We cast it as just std::string rather than std::vector<std::string>

  //There is no "variable" type, which is a problem. I should make a mutating "variable". Problem is user has to parse it properly? I.e. some expect
  //variables to be arrays, some otherwise. I could make two different functlist_items. Probably not the best. Expects a certain type? Of each? 
}





struct functlist_item {
  std::string tag; //tag (name) of this function.
  size_t nargs; //correct # of args for this function.
  std::vector< std::string > args;
  functtype funct; //pointer to actual function to call
  
  
  //Actually has a LIST of hvs? Crap...? Need access to my "target" in each hv though
  std::string execute( const client::STMNT& fs, std::vector< hierarchical_varlist >& hvs, const std::vector< size_t >& my_hv_idxs )
  {
    if( fs.ARGS.size() != nargs )
      {
	fprintf(stderr, "ERROR: in execute of functlist_item [%s]: nargs (%ld) != args of passed STMNT (%ld)\n", tag.c_str(), nargs, fs.ARGS.size());
	exit(1);
      }
    else
      {
	//construct strings from varlists.
	//std::vector< std::string > evaluated_arglist = vars_to_strings( fs.ARGS, std::vector< hierarchical_varlist >& hvs, const std::vector< size_t >& my_hv_idxs );
	
	return funct( fs, hvs, my_hv_idxs );
      }
  }

functlist_item( const std::string _tag, const functtype _funct, const size_t _nargs )
: tag(_tag),
    funct(_funct),
    nargs(_nargs)
  {
    //REV: do nothing
  }
  
};


struct functional_representation
{
  std::vector< functsig > stmntlist;
  
  //Takes some registered functions struct (list?), and the (unrolled) stmnts thing.
  functional_representation( std::vector< client::STMNT >& stmnts, registered_functs& regfunts )
  {
    //Make a linked list or something, since I want to be able to insert...
    //Simple, deconstruct each statement into an unrolled version, and then push it back. All things go on the "top", since if it was nested, ordering
    //shouldn't mattered, since references within a nested function are not allowed (I could check this and error out, but nah not yet)
    //First, unroll all leaf statements up to top, and name them something specific/unique. Need to make new variable/set variable to the output of that
    //funct. So, I DO need variables. But, don't need to set the variable, I just need to add a stmnt to set a new variable to be the output of that
    //function. I.e. we only distil down to "set" and "read" variables. So, I  need to know what that is. I assume I must know the basic functions which
    //are "READ" and "SET". Just check if functsig is one of those. Fine. Make it recursive until end is met...note I don't check validity of functs
    //and arg list lengths etc. until after I do the string parsing and unfolding. Because it requires semantic info like regfuncts.
    for( size_t s=0; s<stmnts.size(); ++s )
      {
	
      }
    
    //Unrolls the stmnts, and also finds their corresponding registered funct in the regfuncts.
    //Note, we do not parse any variable contents yet, we just create the "semantic" skeleton. Thus, I don't need the variable list yet.
    
  }
}

struct pset_functional_representation
{
  size_t nthreads; //number of parallel threads
  std::string myname;
  
  pset_functional_representation( client::PSET& ps )
  {
    myname = ps.NAME;
    //Builds the thing based on that.
    nthreads = std::stoul( ps.NREP );
    
    //Iterate through STMNT, constructing appropriate FUNCTIONAL things for each. Note at each time point, it will use a new value of the THREAD LOOP
    //variable. I guess easiest way is to move all variable references "up" above the function being called. All variables need to reference and/or be
    //SET actively! For "this" worker guy. Note, I can generate the functional representation without actual parsing the variables. I.e. leave
    //variables unreferenced and functions un-run. But, un-nest functions. That is the goal is to un-nest all function, i.e. put them up above.
  }

  std::vector< worker_functional_representation > generate_workers( hierarchical_varlist& hv )
  {
    //Need the hv for what reason? Need to know my "pset" position in here. Ah, for reference purposes, of course.
    std::vector<  worker_functional_representation > wfrs;
  }

  //FUNCTIONAL REP
  
  
  
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


  void build_parampoint( hierarchical_varlist& hv )
  {
    //Runs the function for the stored script and functional representation...
  }
}


//Something will call this. Specifically, the farm_funct
struct parampoint_generator
{
  
  std::vector< hierarchical_varlist > named_vars;

  std::vector< hierarchical_varlist > parampoint_vars;

  //It's a single executable representation...
  executable_representation exec_rep; //representation of the "script" to run to generate psets (param point) etc.
  
  //takes an argument which is just a (new?) variable list of the parameters to test, and does stuff. Note some (other fixed parameters) might be global.
  void generate( const varlist& vl )
  {
    //Takes the varlist...
    hierarchical_varlist hv( vl );
    
    parampoint_vars.push_back( hv );
    
    //Make the children, we need to go through the executable representation
    //This will build the CMD strings down to the worker level.
    exec_rep.build_parampoint( hv );
  }
}
