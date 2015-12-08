



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


template <typename T >
void concat_deque( std::deque< T >& topushto, const std::deque< T >& src )
{
  if(src.size() == 0) { return; }
  
  for(size_t d=src.size()-1; d>=0; --d)
    {
      topushto.push_front( src[ d ] );
    }
}

template <typename T >
void insert_deque( std::deque< T >& topushto, const std::deque< T >& src, size_t loc )
{
  if( loc >= topushto.size() )
    {
      fprintf(stderr, "ERROR in insert_deque, trying to insert outside of size of target queue\n");
      exit(1);
    }

  topushto.insert( topushto.begin()+loc, src.begin(), src.end() );

  return;
}


std::string make_tmpvar ( size_t& varidx )
{
  ++varidx;
  return ( "__TMPLOCAL_" + std::to_string( varidx ) );
}


//REV: WHat about variables that do things like create directories, set required files etc.? Add new required file etc.?
//This is for setvar.
std::deque< client::STMNT > check_atomic_setvar( const client::STMNT& ss, size_t& varidx )
{

  //REV: check what the tag is. If it is a SETVAR, eventually, we need to allow it to have one nested level.
  //But only the second variable should have one nested level.
  
  std::deque < client::STMNT > newlist;
  client::STMNT oldguy = ss;
  
  if( oldguy.TAG != "SETVAR" )
    {
      fprintf(stderr, "ERROR calling check_atomic_setvar on non SETVAR STMNT (is [%s])\n", oldguy.TAG.c_str());
      exit(1);
    }
  if( oldguy.ARGS.size() != 2 )
    {
      fprintf(stderr, "ERROR in special case SETVAR STMNT check atomic: arglength is not == 2 (==[%ld])\n", oldguy.ARGS.size());
      exit(1);
    }

  

  //Check that first argument is ATOMIC (readvar)
  LEAF_STMNT ts = oldguy.ARGS[0];
  if( ts.ARGS.size() != 0 )
    {
      //it's not atomic. Thus, we must make a new tmpvar, set it to this guy as an argument, push it to beginning...
      client::LEAF_STMNT nguy( ts.TAG, ts.ARGS ); //make a new (leaf) statement that is the guy, because it will be an argument to a SETVAR
      std::string tmpvarname = make_tmpvar( varidx );
      client::LEAF_STMNT varn( tmpvarname ); //empty one, i.e. read var
      std::vector< client::LEAF_STMNT > topush;
      topush.push_back( varn );
      topush.push_back( nguy );
      client::STMNT setguy ( "SETVAR",  topush );
      newlist.push_front ( setguy );
      oldguy.ARGS[s] = varn; //this is the "read var" leaf stmnt.
    }

  //Check that second argument is ONE-LAYER-NESTED at most.
  LEAF_STMNT ts2 = oldguy.ARGS[1];
  //Do a check_atomic on THAT statement. Note if it is already a readvar, just leave it. Otherwise, we want to compress it to a first-layer STMNT,
  //but for type stuff we unfortunately need to leave it as LEAF_STMNT. So make one that returns the "new" leaf statement to replace it with, and
  //also statement list to put up above
  //if ARGS.size() == 0, it's just a read, so we're done.

  //If it's > 0, then we must check it, add to above statements, and modify ts2 of oldguy.
  if( ts2.ARGS.size() != 0 )
    {
      client::STMNT tmpstmnt( ts2.TAG, ts2.ARGS ); //construct a STMNT from our LEAF_STMNT (ghetto )
      
      std::deque< client::STMNT > toadd = check_atomic( tmpstmnt, varidx );
      if( toadd.size() < 1 )
	{
	  fprintf(stderr, "ERROR, check_atomic returned something of size <1, error! [%ld]\n", toadd.size() );
	  exit(1);
	}

      client::STMNT orig = toadd [ toadd.size() - 1 ];
      toadd.pop_back( ); //delete the last guy
      
      //reconstruct a leaf stmnt from the updated guy. Actually just set the ts2 index guy to that.
      oldguy.ARGS[1] = client::LEAF_STMNT( orig.TAG, orig.ARGS );

      //Add the returned dequeued guys to beginning of our dequeu. Go backwards
      concat_deque( newlist, toadd );
    }
  
  
  newlist.push_back( oldguy ); //finally add the "sanitized (one level)" guy at the end
  return newlist;
}



//This is for any other function. I.e. all guys must be atomic.
std::deque< client::STMNT > check_atomic( const client::STMNT& ss, size_t& varidx )
{

  //REV: check what the tag is. If it is a SETVAR, eventually, we need to allow it to have one nested level.
  //But only the second variable should have one nested level.
  
  std::deque < client::STMNT > newlist;
  client::STMNT oldguy = ss;
  
  for( size_t s=0; s<oldguy.ARGS.size(); ++s )
    {
      LEAF_STMNT ts = oldguy.ARGS[s];
      if( ts.ARGS.size() != 0 )
	{
	  //it's not atomic. Thus, we must make a new tmpvar, set it to this guy as an argument, push it to beginning...
	  client::LEAF_STMNT nguy( ts.TAG, ts.ARGS ); //make a new (leaf) statement that is the guy, because it will be an argument to a SETVAR
	  std::string tmpvarname = make_tmpvar( varidx );
	  client::LEAF_STMNT varn( tmpvarname ); //empty one, i.e. read var
	  std::vector< client::LEAF_STMNT > topush;
	  topush.push_back( varn );
	  topush.push_back( nguy );
	  client::STMNT setguy ( "SETVAR",  topush );
	  newlist.push_front ( setguy );
	  oldguy.ARGS[s] = varn; //this is the "read var" leaf stmnt.
	}
    }
  newlist.push_back( oldguy ); //finally add the "sanitized (one level)" guy at the end
  return newlist;
}


//RECURSIVE:
//We have a list of statements. Each one can/could be expanded to (replaced by) a larger set of statements. Ideally in place. But it will go deep.
//Just do as a single. For any given vector of statements, it will iterate through, and replace each with the next (depth) guy.
//Just make sure to not modify for loops while I go through them.
std::deque< client::STMNT > recursive_unroll_nested_functs( std::deque< client::STMNT >& stmnts, size_t& varidx )
{
  std::deque <client::STMNT> locals = stmnts;
  bool finished = false;
  while( !finished )
    {
      finished=true;
      
      for(size_t s=0; s<locals.size(); ++s)
	{
	  client::STMNT st = locals[s];
	  std::deque< client::STMNT > tmpvec;
	  tmpvec.push_back ( st );

	  std::deque< client::STMNT > retval = recursive_unroll_nested_functs( tmpvec, varidx );

	  //If it returned only a single value, it means that it was recursively unrolled already
	  if(retval.size () < 1 )
	    {
	      fprintf(stderr, "ERror returned val from recurisve unroll was 0, \n"), exit(1);
	    }
	  
	  if( retval.size() == 1 )
	    {
	      //do nothing.
	    }
	  else
	    {
	      //Insert them all to the beginning of S location, and restart s loop. Note we must delete S...?
	      locals.erase( locals.begin() + s );

	      insert_deque( locals, retval, s );
	      //concat_deque( locals, retval ); //pushing to beginning. I *could* do an insert in place...which might actually be better heh.
	      
	      finished = false;
	      break; //this should break the for loop, but not the outer while loop
	    }
	}
    }
  
  return locals;
}


struct functional_representation
{
  std::vector< functsig > stmntlist;

  //Takes some registered functions struct (list?), and the 
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
