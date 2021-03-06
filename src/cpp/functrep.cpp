

#include <functrep.h>

  functsig::functsig()
  {
  }
  
  functsig::functsig( const std::string& _tag, const size_t& argsize, const functtype& f )
:
  mytag(_tag),
    nargs( argsize ),
    funct( f )
  {
    //Do nothing.
  }



FUNCTDEF( GET_PREV_VAR )
{
  //NOTE root down 1 is pset #
  size_t targpset = std::stoul(args[0].get_s());
  std::vector<size_t> c = hvl[0].get_children( 0 );
  if( targpset >= c.size() )
    {
      fprintf(stderr, "ERROR trying to access PSET that doesn't exist (yet?) in GET_PREV_FILE. Requested [%ld] but only have #children psets [%ld]\n", targpset, c.size() );
      exit(1);
    }
  size_t psetidx = c[targpset];
  std::vector<size_t> c2 = hvl[0].get_children( psetidx );
  size_t targpitem = std::stoul(args[1].get_s());
  size_t pitemidx = c2[targpitem];

  variable<std::string> newvar( hvl[0].getvar( args[2].get_s(), pitemidx ) );
  return newvar;

  //Arg1 is PSET #
  //Arg2 is WORKER #
  //Arg3 is FILENAME (?) (i.e. variable name IN THAT DIR we want to get?)
  //What if we don't care about looking in HIERARCH index, but want to access a file in its DIRECTORY. I.e. we need a way to tell it to prepend
  //   that PSET/PITEM directory to the string/filename I give. Ah.
  //Do all contain a "dir" name that tells dir they are in?
  //Special function to return MYDIR.
}

FUNCTDEF( GET_PREV_DIR )
{
  std::vector<variable<std::string> > args2 = args;
  args.push_back( variable<std::string>("__MY_DIR", "__MY_DIR") );
  return GET_PREV_VAR(args, hvl, hvi );
}

//Need a way to specify where to write out stuff.
//Need a way to specify every variable as applying to model or what?
//Just write the whole thing out there? I.e. some list of variables to write out automatically.
//User variable __INPUT_FILE is the file where everything is printed to. Note we should automatically put it somewhwere? This is
//different than __REQUIRED_FILES which might not be stored in memory (e.g. sound file, data file input from experimental data, etc.).
//But in files that are specified. They are immutable. We store them once per worker thread and give those access.

//So, START TIME data file. We specify those "per worker" things at beginning not of PSET, but of the whole program.
//E.g. "use TMP" or something. And specify location of TMP on each system. Do we really need to copy it N times? Only copy it once in the
//HDF5? No, input data might be too large...but better to have a record of it. So TAR it up or something and put it in a place? Meh too big. Just
//leave it out... It will still take HASH of each and compare it at the beginning of each, or regularly?
//Yea, system CMD takes too long, definitely set it up so that I can specify the MODEL TO CALL or something. Specify dynamic name of function to call?
//Figure that out later. I can't do it as a script in that case I assume, because I'd have to read out a string. But user could specify line of
//function arguments? But type would have to be specified too in that case (haha.), and so user would have to write C++ to handle it...i.e. his
//specific model. Whereas now I don't need it.

//I would really like to "keep" all memory/model stuff around so I don't have to re-allocate it every time... Generating stuff on the GPU is
//probably most efficient. And reading out (every N time steps or something). Storing data on GPU in buffer. Do asynchronous.
//Go through each compute e.g. synapse model "group" separately? If we can't fill up the CPUs, run multiple at a time.

FUNCTDEF( ITER_ARRAY_WITH_FLAG )
{
  //REV: ITERATE through array, interspacing with flag each time. Should
  //return another array variable in the end

  //These will error out if not right variant type.
  //make another for array-array
  std::string FLAG = args[0].get_s();
  //std::vector< std::string > arr = args[1].get_v();
  variable<std::string> arr = args[1];
  //fprintf(stdout, "EXECUTING ITER WITH FLAG, casting to array...\n");
  std::vector<std::string> arr2 = arr.get_v();
  std::vector< std::string > out;
  for(size_t x=0; x<arr2.size(); ++x)
    {
      out.push_back( FLAG );
      out.push_back( arr2[x] );
    }

  //fprintf(stdout, "FINISHED MAKING NEW ONE IN ITER_WITH_FLAG\n");
  variable< std::string > new_arrayvar( "__ITER_ARRAY_WITH_FLAG_RETVAL", out);
  return new_arrayvar;
}

FUNCTDEF( GET_OUTPUT_FILES )
{
  //RETURN ARRAY (output files)
  return hvl[0].getvar( "__MY_OUTPUT_FILES", hvi[0]);
  //std::vector< std::string > dv = { "__DUMMYVALUE1"};
  //return variable<std::string>("__DUMMYOUTPUTFILES", dv );
  //return variable<std::string>("__DUMMYOUTPUTFILES", "__DUMMYOUTPUTSTRING" );
}

FUNCTDEF( SET_INPUT_FILE )
{
  //hvl[0].setvar( "__MY_INPUT_FILE", args[0].get_s(), hvi[0]);
  hvl[0].setvar( "__MY_INPUT_FILE", args[0], hvi[0]);
  return variable<std::string>("__SETINPUTSUCCESSNAME", "__SETINPUTSUCCESSVAL"); 
}

FUNCTDEF( GET_INPUT_FILE )
{
  return hvl[0].getvar( "__MY_INPUT_FILE", hvi[0]);
}


FUNCTDEF( VARLIST_TO_FILE )
{
  //REV: print current (my?) varlist to a target file?
  //Or is it the root or something? This is super ugly. Is that the only way to transfer my varlist to them?
  //Or, automatically print it in PARAMPOINT? And reference that? Might have separate guys trying to access it
  //individually though...
  
  //Remember, in extreme cases we will have everything written to TMP locally, then copied (mv?) to appropriate position.
  //Then it will be written to HDF5?
}

FUNCTDEF( ADD_SUCCESS_FILE )
{
  hvl[0].add_to_var( "__MY_SUCCESS_FILES", args[0].get_s(), hvi[0] );
  return variable<std::string>("__ADDEDTOSUCCESSNAME", "__ADDEDTOSUCCESSVAL"); 
}

FUNCTDEF( ADD_REQUIRED_FILE )
{
  hvl[0].add_to_var( "__MY_REQUIRED_FILES", args[0].get_s(), hvi[0] );
  return variable<std::string>("__ADDEDTOREQUIREDNAME", "__ADDEDTOREQUIREDVAL"); 
  
}

FUNCTDEF( ADD_CMD_ITEM )
{
  //REV: adds one at a time?
  //fprintf(stdout, "((( ADD CMD ))) ADDING TO CMD: [%s]\n", args[0].get_s().c_str() );
  hvl[0].add_to_var( "__MY_CMD", args[0].get_s(), hvi[0] );
  return variable<std::string>("__ADDEDTOCMDNAME", "__ADDEDTOCMDVAL"); 
  
}

FUNCTDEF( ADD_CMD_ITEMS )
{
  //REV: adds one at a time?

  hvl[0].add_array_to_var( "__MY_CMD", args[0].get_v(), hvi[0] );
  
  return variable<std::string>("__ADDEDTOCMDNAME", "__ADDEDTOCMDVAL"); 
  
}

FUNCTDEF( CAT_ARRAY_TO_STR )
{
  //std::string

  //arg 0 is array var (unknown length, we will iter through)
  //arg 1 is SEPERATOR (sep), which is char/str that will separate everything together (?).
}

FUNCTDEF( ADD_OUTPUT_FILE )
{
  hvl[0].add_to_var( "__MY_OUTPUT_FILES", args[0].get_s(), hvi[0] );
  return variable<std::string>("__ADDEDTOOUTPUTNAME", "__ADDEDTOOUTPUTVAL"); 
  
}

//ARRAY_TO_STRING_WITH_SEP
//FORLOOPNTIMES
//Super nice to have a way to check file fullness/length/conformity etc.. haha. Instead of just existence?

//Crap, this should only setvar in the PSET varlist!
FUNCTDEF( SETVAR )
{
  //better have args length == 2, otherwise we would throw this away with an error already
  //name, contents
  //Which hvl do I set? Are there multiple that I have access to? Make sure some are "named" and others "pset". Give user way to specify which tag to set
  //it to.
  //Set the specified varlist in the 0th hierarchical varlist passed to the value specified.
  hvl[0].setvar( args[0].get_s(), args[1], hvi[0] );
  return variable<std::string>("__SETVARNAME", "__SETVARVAL");

  //modifies state, no return...!
}



FUNCTDEF( IDENTITY )
{
  //fprintf(stdout, "INSIDE IDENT FUNCT. Size of args is [%ld]\n", args.size() );
  //myvar_t test("TMPVAR", "YOLO");
  //return test;
  return (variable<std::string>("__TMPVARNAME(IDENT)", args[0].get_s() ) );
}

FUNCTDEF( GET_MY_DIR )
{
  //fprintf(stdout, "INSIDE IDENT FUNCT. Size of args is [%ld]\n", args.size() );
  //myvar_t test("TMPVAR", "YOLO");
  //return test;
  return ( hvl[0].getvar("__MY_DIR", hvi[0]) );
}

FUNCTDEF( READVAR )
{
  if(hvl.size() < 1)
    {
      fprintf(stderr, "ERROR hvl.size < 1\n"); exit(1);
    }
  if(args.size() < 1)
    {
      fprintf(stderr, "ERROR args.size < 1\n"); exit(1);
    }
  if(hvi.size() < 1)
    {
      fprintf(stderr, "ERROR hvi.size < 1\n"); exit(1);
    }
  return ( hvl[0].getvar( args[0].get_s(), hvi[0]) );
}


FUNCTDEF( CAT )
{
  //Takes N args? Or, takes just 2? Takes a list? Of variable names (wow?)
  //How do I add strings??? Shit...at some point I have to add strings! Or literals. Crap...
  //Crap, fuck, shit. It means that LEAF_STMNT can be literals. Which means I need to parse them. They don't have the () ending.
  //And I need to deal with closing parens etc. Always enclose literals in quotes or something? Or make "read" variables use $() or something?
  //Ghetto shell script language like thing haha.
  //REV: OK, so do "X" is literal, "$X" is variable read, etc.
  //Add a literal tag for it?
  
  std::string sep=args[0].get_s();
  std::string c = args[1].get_s() + sep + args[2].get_s();
  //Variables must always have names?
  variable<std::string> v( "TMPVAR", c );
  return v;
}


functsig functlist::findfunct( /*const*/ client::STMNT& s )
  {
    //artificially convert to a READVAR. Ghetto, I know ;)
    
    //1) A literal (if it has no args and does not begin with a $, ISLIT is true)
    //2) A readvar (if it has no args and begins with a $, ISLIT is false)
#if DEBUGLEVEL>5	
    fprintf(stdout, "Searching for function [%s] (from STMNT)\n", s.TAG.c_str() );
#endif
    for(size_t x=0; x<sigs.size(); ++x)
      {
	//Match tag to sigs[x]
	if( s.TAG == sigs[x].mytag )
	  {
	    if( s.ARGS.size() != sigs[x].nargs )
	      {
		fprintf(stderr, "ERROR: in findfunct, found function matching tag [%s], however #args of STMNT (=%ld) != #args expected by function signature (=%ld)\n", s.TAG.c_str(), s.ARGS.size(), sigs[x].nargs );
		exit(1);
	      }
	    else
	      {
#if DEBUGLEVEL>5	
		fprintf(stdout, "In findfunct: FOUND correct function, returning function signature [%ld] (name is [%s], takes [%ld] args, function pointer is...)\n", x, sigs[x].mytag.c_str(), sigs[x].nargs);
#endif
		//std::cout << sigs[x].funct << std::endl;
		return sigs[x]; //.funct;
	      }
	  }
      }
    fprintf(stderr, "REV: ERROR: findfunct could not find function with tag we are searching for [%s] (searched through [%ld] function signatures)\n", s.TAG.c_str(), sigs.size() );
    exit(1);
    
  }

  
  
  functsig functlist::findfunct( /*const*/ client::LEAF_STMNT& s )
  {
#if DEBUGLEVEL>5	
    fprintf(stdout, "Searching for function [%s] (from LEAF_STMNT)\n", s.TAG.c_str() );
#endif
    //client::LEAF_STMNT s2 = s;
    
    if( s.ISLIT )
      {
#if DEBUGLEVEL>5	
	fprintf(stdout, "(In search: It's a literal!)\n");
#endif
	if( s.ARGS.size() > 0 )
	  {
	    fprintf(stderr, "REV: ERROR, found an ISLIT with >0 arguments!\n");
	    exit(1);
	  }
	std::string FNAME="ERROR";
	if( s.TAG[0] == '$' )
	  {
	    FNAME="READVAR";
	  }
	else
	  {
	    //make the funct directly? Have one there that takes a new STMNT and makes tag?
	    //FNAME="IDENTITY"; //Identity takes the first empty LEAF and uses tag as raw string (makes var from it)
	    //return functsig( s2.TAG, 0, nullptr );
	    return functsig( s.TAG, 0, nullptr );
	  }
	std::vector< client::LEAF_STMNT > tmplist;
	std::vector< client::LEAF_STMNT > tmplist2;
	//client::LEAF_STMNT tmp( s2.TAG.substr(1), tmplist2, true ); //to std::npos
	client::LEAF_STMNT tmp( s.TAG.substr(1), tmplist2, true ); //to std::npos
	tmplist.push_back( tmp );
	//it must be a variable read.
	s = client::LEAF_STMNT( FNAME, tmplist, false ); //its now a READVAR with arg. I.e. not lit.
	
      }
    
    client::STMNT tmps( s.TAG, s.ARGS );
    
    //REV: can I do this or will it complain because I'm constructing it temporarily here?
    return findfunct( tmps );
    //as above...
  }
  
  functlist::functlist()
  {
    ADDFUNCT( 1, IDENTITY );
    ADDFUNCT( 1, READVAR );
    ADDFUNCT( 2, SETVAR );
    ADDFUNCT( 3, CAT );
    ADDFUNCT( 1, GET_PREV_VAR );
    ADDFUNCT( 1, GET_PREV_DIR );
    ADDFUNCT( 1, SET_INPUT_FILE );
    ADDFUNCT( 0, GET_INPUT_FILE );
    ADDFUNCT( 1, ADD_SUCCESS_FILE );
    ADDFUNCT( 1, ADD_REQUIRED_FILE );
    ADDFUNCT( 1, ADD_CMD_ITEM );
    ADDFUNCT( 1, ADD_CMD_ITEMS );
    ADDFUNCT( 2, CAT_ARRAY_TO_STR );
    ADDFUNCT( 1, ADD_OUTPUT_FILE );
    ADDFUNCT( 0, GET_MY_DIR );
    ADDFUNCT( 2, ITER_ARRAY_WITH_FLAG );
    ADDFUNCT( 0, GET_OUTPUT_FILES );
    
      /*functtype fa = IDENTITY;
	functtype fb = READVAR;
	functtype fc = SETVAR;
	functtype fd = CAT;
	ADDFUNCT(a, "IDENTITY", 1, fa );
	ADDFUNCT(b, "READVAR", 1, fb );
	ADDFUNCT(c, "SETVAR", 2, fc );
	ADDFUNCT(d, "CAT", 3, fd );
	
	
    //USERS: Add other function signatures here
    sigs.push_back( a );
    sigs.push_back( b );
    sigs.push_back( c );
    sigs.push_back( d );
    
    }
      */
      
  }

  //A functrep could be JUST a simple VARIABLE. Need to always parse down to a literal.
  //Problem is for function IDENTITY only, we just return here.
  
  //REV: crap, it possibly will take a LEAF_STMNT too? Just use another one.
functrep::functrep( client::STMNT& s, functlist& fl )
  {
    //Functs to not have types, arguments do not have types. Only # of arguments orz.
    //REV: this will check # of arguments etc. It will use s.TAG to get function, and all that.
#if DEBUGLEVEL>5	
    fprintf(stdout, "Constructing a funct rep from stmnt: \n");
#endif
    //REV: Just leave IDENTITY functions as such. I.e. create
    //REV: WE NEED IT TO BOTTOM OUT IN A FUNCTREP THAT JUST RETURNS THE STRING. Note, the functrep will simply return itself (functsig tag), it will not
    //actually call "identity".
    //We signify this by leaving funct pointer as nullptr?! Sounds good...
    
    fs = fl.findfunct( s );
    for( size_t nesti=0; nesti<s.ARGS.size(); ++nesti )
      {
#if DEBUGLEVEL>5	
	fprintf(stdout, "In functrep with STMNT constructor, doing [%ld] arg (%s)\n", nesti, s.ARGS[nesti].TAG.c_str());
#endif	
	functrep tmpfr = functrep( s.ARGS[nesti], fl );
	args.push_back( tmpfr );
      }
  }

  //Constructor that takes leaf stmnt instead. Whatever.
  functrep::functrep( client::LEAF_STMNT& s, functlist& fl )
  {
#if DEBUGLEVEL>5
    fprintf(stdout, "Constructing a funct rep from leaf_stmnt: \n");
#endif
    fs = fl.findfunct( s );
    //REV: does this not take into account the changed s! I.e. $AAAA -> READVAR AAAA
    for( size_t nesti=0; nesti<s.ARGS.size(); ++nesti )
      {
#if DEBUGLEVEL>5
	fprintf(stdout, "In functrep with LEAF STMNT constructor, doing [%ld] arg (%s)\n", nesti, s.ARGS[nesti].TAG.c_str());
#endif
	functrep tmpfr = functrep( s.ARGS[nesti], fl );
	args.push_back( tmpfr );
      }
  }

  functrep::functrep( functsig f )
  {
    fs = f;
  }
  
  void functrep::enumerate()
  {
    //for(size_t s);
  }
  
  //std::vector< hierarchical_varlist >&, const std::vector< size_t >&)
  //Executes and makes the variable output of this guy.
  //Note it needs access to the hierarchical varlists to use.
  //Wait, it takes a list of variables now, at the lowest level I guess.

  //REV: we do not actually resolve to VARIABLE types until now (execution time)
  myvar_t functrep::execute( std::vector< hierarchical_varlist<std::string> >& hvl,  const std::vector< size_t >& hvi )
  {
    //It's a literal!
    if( fs.funct == nullptr )
      {
#if DEBUGLEVEL>5
	fprintf(stdout, "GOT A LITERAL (%s)\n", fs.mytag.c_str());
#endif
	return myvar_t( "TMPVARNAME(LIT)", fs.mytag );
      }
    else
      {
	std::vector< myvar_t > mv;
	//Converts the leaf statement to a myvar_t


	//ARGS was set to zero here!
	for( size_t x=0; x<args.size(); ++x ) //This is the nested STMNT. I need to convert those to VARIABLES (via user functions) to actually do things.
	  //But, what about if a function is e.g. "construct variable from args" type thing, that constructs an array or composes one? Or what about one
	  //that builds a large string? User function needs to e.g. handle parsing to integer, blah. We do that but then still return to myvar in the end.
	  {
#if DEBUGLEVEL>5
	    fprintf(stdout, "Executing (recursively) the [%ld] argument (it holds [%s] and has [%ld] args)\n", x, args[x].fs.mytag.c_str(), args[x].args.size() );
#endif
	    //REV: WTF? Wait, I need user to specify how to individually handle each thing into a variable? No, into actual STRING at the end!
	    //Each args[x] will have its own set of blah?
	    //Note, none of these guys should update actual things?
	    //Note some variables may return empty things (if they're empty? No, they won't be in here...)

	    //PROBLEM: Myvar should not be a "set" in an argument though.
	    //We might have "empty" variable returned in some conditions.

	    //REV: huh, it's now "executing" the argument
	
	    
	    mv.push_back( args[x].execute( hvl, hvi ) );
	  }
	//execute ft and recursively do so.
#if DEBUGLEVEL>5	
	fprintf(stdout, "Attempting to execute actual functional (%s)! #ARGS:[%ld]\n", fs.mytag.c_str(), mv.size());
#endif
	/*for(size_t x=0; x<mv.size(); ++x)
	  {
	  }*/

	//MV is of size ZERO here!
	
	myvar_t ret = fs.funct( mv, hvl, hvi ); //however, this might return an "empty" variable if it just has a side-effect...
	//fprintf(stdout, "(in execute: ) GOT RET: [%s]\n", ret.get_s().c_str());
	return ret; //This may return e.g. CMD. Or final side effect may be to set CMD to it? Some special lowest-level variable named CMD?
      }
  }
 
