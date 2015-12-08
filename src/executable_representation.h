//REV: ORGANIZATION:
//There are two hierarchical variable lists.
//
//1) One is a general "named" variable list. Which will have a "flat" list of variable lists, which may be
//per-model etc.
//
//2) The other is the param-point varlist, which is a list, but it is hierarchical, for the purpose of e.g.
//a GENERATIONS
//b PSETS
//c WORKERS

//And, what is actually passed is any of the guys, and it will go up to parents.
//How about just making a single structure. It has the named side, and it has the parampoint side. Which will contain all the param point lists?
//Will it store all previous parampoints as well? No...each param point is a new one, OK. Why not just make them all part of one, and parampoints are
//just some simple guy..? Hm, nah. How to specify global parameters we want to read, and which not to? Via "namespace" type guys? If we give mytag
//type string names to the guys...hm. Realistically, how will I use it? At any rate, I need the way to search for a variable, given the guy.



//REV: User functions will NECESSARILY know how to use/parse targets. Only user defined "bound" functions such as those to distribute -b X -b N
//etc will be able to use variables...? When reading variables, it will always only convert to a single string. During reading, if user tries to read
//an array raw, that will cause an error of course... He must always convert to a string somehow first? Some default tells what separator to use.
//OK. So, now I have defined hierarchy. I need to tell "which" of the varlist to search in. One is hierarchical.
//Note, one is root...? It's "always" a list of hierarchical varlists?


#pragma once

#include <cstdio>
#include <vector>
#include <cstdlib>
#include <string>


#include "variable.h"



//REV: OK, so now what I have is the functions etc. that does executable representation, and they will "create" the appropriate varlist etc., based
//on "parampoint" etc.?
//Note, there are some "functional" variables, such as file directory, etc.? Worker number?

//Let user add to these, etc. How do we do "named" vars? How do we access "worker X in previous paramset etc.?"



std::vector< hierarchical_varlist > named_vars;

std::vector< hierarchical_varlist > parampoint_vars;





//Need a way to search through these, based on name, contained guy etc. Assume each thing has like an "included" list of the guys it will search through.
//Um, need a way to specify for "which" paramter point this storage thing is for. Is it better to keep the actual varlists around? Store them in some
//pickled struct? Or what? For user, it depends on his "idea" of generations. What if I'm doing a grid search for example, all are known at beginning.
//I don't make them all, I do them as I compute them. So, there is a "lifetime" of them? Or I just keep growing them in memory until I die lol. Much
//better to have "above!" and "below", and if I try to modify one above, it's an error. Or something. I.e. can'T modify if it has any children? Fine.

//So, variables are set and stuff I guess? Need to add a "hierarchy" to them though. Easier to literally do it, so deleting top will delete bottom.
//But, if I add hierarchical, I need a way to search up above me, i.e. I need pointer "up", which is a pain in the butt. So, I should make a wrapper
//struct of some course, that searches... Hm, not such a pain in the butt I guess, though it would need to use nasty "pointer" type things? Better to
//wrap it so I can do things like know about parampoint, psets, and workers?



//REV: I like this because it saves me by giving me a way to include variables as strings in memory! I.e. for that parameter point in general. And we can
//have more general global variables at an even higher level.


//Problem is if some variables/stuff is GENERATIVE, i.e. based on algorithm/procedure. In which we have to also specify the procedure to generate, the
//specific details of what we used. Meh, forget that for now though.

//So, do the param point things. Great. And have some "global" level guys too. But, those might be "named" things, i.e. not model specific? Each pset/model
//might want "different" variable sets. Hm. In which case, we're reading from named guys anyway (they are shared model guys). So, we can import those
//automatically? Or do like "include model: BLAH". Basically, a namespace lol. OK, so make the other guys parameterized... by parampoints. Fine.


//Right, there's 2. What do I need to code? I always search the two? 




//REV: what is the point of this? To give the "context" for variables...I forgot lol. Always search from a lowest level. Or from arbitrary node. OK.
//Does it (inefficiently) construct a new varlist that is flat for each one? And I add in "namespace" type stuff?
struct hierarchical_varlist
{
  //Contains a list of varlist and their relationship? And also some functions to help me search them. Much better to make one that is specific for
  //param points etc.

  //linked list of guys? How to "find" param point varlists?
  //Explicitize our levels?

  std::vector< varlist > vl;

  //Linked list might be way better. Using pointers etc...meh.
  //Note, these are VARLIST indices, not individual variables...
  std::vector< std::vector< size_t > > children;
  std::vector< size_t > parents; //who is the parent of root "0"? There is none.
  
  //How to reference a varlist in here? "which" child of which one? Root is always #0
  //Crap, if I use this instead of indices, we have a problem... Most will use from leaf to top... Hm, yea seems definitely better to literally build
  //the structure inside it...
  //It's called a heap? Or a tree? Use STL or something? What is "contents". There are no STL tree structures because too many options apparently...
  //Just a GRAPH. How do I expect to use it? Does user have a pointer to a single varaible list, and it is connected to others in an "include" like
  //manner? For a given example, we can construct a "temporary" full varlist by concatenating all the ones the user requests. And/or we could just
  //search them all I guess.
  //This is just used internally.
  const size_t get_parent( const size_t v )
  {
    if(v == 0)
      {
	fprintf(stderr, "ERROR in get_parent, trying to get parent of root...\n");
	exit(1);
      }
    else if( v < parents.size()-1 ) //bc root has no parent...
      {
	return parents[v-1]; //pain in the butt.
      }
    else
      {
	fprintf(stderr, "ERROR IN get_parent: tried to get [%ld]-th member of parent vector, but its out of bounds (vector is [%ld]-length)\n", v, parents.size());
	exit(1);
      }
  }

  const std::vector<size_t> get_children( const size_t v )
  {
    if( v < children.size() )
      {
	return children[v];
      }
    else
      {
	fprintf(stderr, "ERROR IN get_children: tried to get [%ld]-th member of children vector, but its out of bounds (vector is [%ld]-length)\n", v, children.size());
	exit(1);
      }
  }
  
  void add_child( size_t parent, const varlist& v )
  {
    if( parent >= vl.size() )
      {
	fprintf(stderr, "Error trying to add to a parent that doesn't exist...use add_root()\n");
      }
    vl.push_back( v );
    size_t myindex = vl.size() - 1 ;
    children[ parent ].push_back( );
    children.push_back( std::vector<size_t>(0) );
    parents.push_back(parent);
  }

  void add_root( const varlist& v )
  {
    vl.push_back( v );
    children.push_back( std::vector<size_t>(0) );
  }

  
  
  hierarchical_varlist()
  {
    //There is nothing... must use 
  }

  hierarchical_varlist(const varlist& start_root)
  {
    vl.push_back( start_root ); //no need to push back parent, there are none. Push back children I guess.
    children.push_back( std::vector<size_t>(0) );
  }

  //FUNCTIONS TO SEARCH FOR VARIABLES IN A HIERARCY, I.e. going through to parents until the root.
  //What "argument" do I take to specify start-point though? Um, just use index for now? And I can search for a pointer in the worst case rofl?
  //Ah, mytag exists...? We can construe them? Or use like "nth" of mything. Mmmm. PARAMPOINT is a general ideal though. Hm, these are all modified by
  //our thing. But we want to be able to generally create strings right. OK. How do we, for a given worker type guy, know the var thing to use? Give
  //indices in both varlists?

  //REV: returns what? Getarray or getvar. Um, user needs to get a variable. Returns a varlist pointer (index?) and an index inside that. OK. And,
  //what do I do with that?
  //This will actually push the varlists?! No good. Returns their INDICES in here.
  size_t find_var_in_hierarchy( const std::string& varname, const size_t startvl_idx, std::vector< size_t >& vl_indices, std::vector< size_t >& v_indices )
  {
    size_t searchi=startvl_idx;
    while ( searchi != 0 )
      {
	std::vector<size_t> namelocs = vl[searchi].getname( varname );
	for(size_t x=0; x<namelocs.size(); ++x)
	  {
	    vl_indices.push_back(searchi);
	    v_indices.push_back(namelocs[x]);
	  }
	searchi = get_parent( searchi );
      }

    if( searchi != 0)
      {
	fprintf(stderr, "REV: ERROR, getvar, searchi is not zero at end of var search chain...\n");
	exit(1);
      }

    //get for root.
    std::vector<size_t> namelocs = vl[searchi].getname( varname );
    for(size_t x=0; x<namelocs.size(); ++x)
      {
	vl_indices.push_back(searchi);
	v_indices.push_back(namelocs[x]);
      }
    
    
    return v_indices.size();
  } //end getvar

  //REV: why am I not using pointers -_-. Way easier...No guarentee that they will always point to SOMETHING legal? Can I use smart pointers?
  
  std::vector<std::string> get_array_var( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, std::vector< size_t >& vl_indices, std::vector< size_t >& v_indices );

    if( nfound > 1 )
      {
	fprintf(stderr, "REV: ERROR: get_array_var in hierarchical: more than one instance found! (Var=[%s])\n", targ.c_str());
	exit(1);
      }
    else if( nfound == 0 )
      {
	fprintf(stderr, "REV: ERROR: get_array_var in hierarchical: no instances of target var found! (Var=[%s])\n", targ.c_str() );
	exit(1);
      }
    else
      {
	//wasteful, this is re-searching...
	return (vl_indices[0].getarray( targ ) );
      }
    
  }

  std::string get_val_var( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, std::vector< size_t >& vl_indices, std::vector< size_t >& v_indices );

    if( nfound > 1 )
      {
	fprintf(stderr, "REV: ERROR: get_val_var in hierarchical: more than one instance found! (Var=[%s])\n", targ.c_str());
	exit(1);
      }
    else if( nfound == 0 )
      {
	fprintf(stderr, "REV: ERROR: get_val_var in hierarchical: no instances of target var found! (Var=[%s])\n", targ.c_str() );
	exit(1);
      }
    else
      {
	//wasteful, this is re-searching...
	return (vl_indices[0].getvar( targ ) );
      }
    
  }
  //Search for a target variable in a hierarchy (given a source "leaf" or any pointer inside here? Making it way too complex?)
  
}; //end hierarchical varlist



//User writes his own functions, which are void functions that just take a functsig as argument, and runs a specified function.


//REV: what var?
std::string setvar(const functsig& fs)
{
  //What is this function? A global function for setting a global variable list? I really should have a more local one...? Some way of specifying "which"
  //variable to set? Which may be latent?
  SETVAR( fs.args[0], fs.args[1] );
}


//Whoa, sometimes it needs to return something? I.e. a string? Oh shit, it always does...?
std::string getvar(const functsig& fs)
{
  return GETVAR( fs.args[0] );
}


struct functsig
{
  std::string tag;
  std::vector<std::string> args; //args.size() is number of args.
};


//REV: make it also implicitly take a "variable context" or something, so we know which variable set to draw from?
//I.e. take it from "current workspace", or, specify a hierarchical level to take it from? By parsing the variable? I.e. variable name info contains
//the required info? That is a really ugly way to do it? But how else? Will it always read from blah? Nah, might refer to something set/used by user?
//What happens when we set a local variable? Do we want to be able to set remote variables too?
//In the end, everything modifies variables? What's the point of this, it's just playing with setting/reading. It's not actually doing anything.
//The whole point was to make it easier to construct things...and to do stuff like read fitness etc... But some vars have given usages in terms of
//e.g. the search function's variables, on what to read? Setting "required" files to start, etc., that is useful I guess. At any rate, I need to be
//able to read/set variables. Question is, do I allow myself to do it in parallel or not? I.e. how much hierarchical variable sets to do we have?
//All sets above me are visible (of course). Without any reference. but I need a way to refer to variables of certain models specifically? I.e. there
//is a global guy, but what? Where am "I" though? I.e. at time of reading, how do I determine which "set" I'm in? User can set vars e.g. at command
//line or something... Or in that set? How about if I'm not in a pset? Can I have variables outside of it? Ah, yea, what if I'm not in a pset? Do they
//stay for the whole time? Hmmmmmm yea, I guess so... like, global. What if I want to include multiple files? Append them all I guess. Being inside
//"scope". Man scope is a pain in the butt... Just make a temporary variable set, set those values, etc.. Propogate all higher-order guys I guess...

//Refer to previous set guys, etc. I.e. a variable, but it refers to a specific blah? Refer to a previous VARIABLE SET? I.e. the variable set of
//previous worker or pset or blah, and #10. How to refer to different "models" then. User can refer to models I guess. And then when doing variables,
//refer to model as well? Or just have a "global" one too, to make it messy haha. How to refer to it? Use e.g. :: notation? Or, force functions to look
//into e.g. VAR(MODELNAME, VARNAME). Easiest to implement since I don't have to descend into string. I.e. NAME variable sets in other word. How do I
//refer to it if it is too deep though? E.g. what if I want to refer to a previous variable in a workerset type thing? Search all models for name. Or
//just have global model space. Better/easiest to have no "hierarchy". Do I want to be able to specify sub-models? Nah, just give arbitrary names I guess.
//Question is, to find a model that is nested, I need to do multiple e.g. A::B::thing...annoying. So, only allow 1 level of depth? Or tell it which
//variable sets to "use"? I.e. which to "bring in". Hm ok. How about stuff like hm? Yea, I want to be able to read from previous guys at the worst case.
//Give functions for that I guess. And when I set in cmd, just set models, and some other vars, that script will read online. I.e. "modelparams" type shit.
//Like SWEEP:NGEN, and stuff? Want to set user stuff. Ah, it's directly set there though. And in normal C code for that model, user reads that stuff
//anyway, OK. Ah, but it must be named some special way, i.e. user knows it is in SWEEP:BLAH, not just BLAH variable. OK...fine. So, no hierarchy, but
//rather something like "include", which includes all models by default? And when I want to refer to something? I refer to it above? How about psets,
//inside a parameter point? Can I refer to other parameter points? E.g. Nth pset, blahth variable? Special variables? Like, working dir? Mostly for
//required files. Like "output" of previous. Define some functions?

//So, a global variable set, or just one that's inside this thing. Easiest that way, if they can all communicate. Have functions to search globally. And
//also have functions to search more locally? Fine. At any rate, have "tag" for each variable set, and just have a global one? Some may not have
//tags...fine. What is "no-tag". What about ones for each like, one for each guy. What is it a number or something? Make a local one and set stuff
//in there, and then scope disappears. What about global one for the file or something? Nasty...We need to keep those alla round of course. But,
//named in which way? Do I really want to keep one around for each parameter point, forever? I Have no need to really communicate between paramter
//points right? OK, in which case um, have local inside parameter points? Ah, each parameter point also has its own thing. And it is automatically
//set up into a list of "PSETS", and within those, ordinal numbers. OK, and those are accessed in those way. And then, there is general models too.


//So:
//GENERAL (no name)
//NAMED (i.e. model name etc.)
//PARAM POINT / PARAM SET (ordinal in order) / WORKER SET ORDINAL (thread #)

//They have some "pre-defined variables", e.g. for directory, etc. And then there are required files. I set those, adn those define the
//things that other guys require for actually running work. I can set them from cmd line locally, or from config files, etc. Many ways...ghetto.
//Need a "nicer" parser, but whatever, I can do errors and stuff later.

//User always determines how to parse a variable.

//Can variables be arrays? This is a major question. What will I use an array for? To generate a list of numbers for example. A list of files. OK.
//that'S right. E.g. for example if I want to interact with that list, I will make a guy to generate a string from it. But the variable itself is not
//that, it is literally just an array of strings. And then to actually use that I need to apply the function to it and the -b or -v or whatever?

//Should I literally make it nested, or just have connections? I.e. how to tell that it is a hierarcy. Easiest to just have "children" and "parents"
//Where children is a list of that sort. For searching, no need to do that, sort by name or something though I guess. How about having two lists? The
//NAMED/GENERAL one as one, and the PARAM POINT one as another.

//That's fine. OK, so nested, or connections? Easiest to just have a list, one for each param point. Of the current step? And each one is passed that
//parampoint as it. Just a list of parampoint varlists, and they are organized/searchable in some way? They are cleared after each? After they are used.
//Mmmm, so a (doubly) linked list or something. For easy removal after the end of their period. Cycle through them? Just have a list/new one made at
//each time point? A way to access them in some "specific" way? I need to pass a pointer to one for each "parameter point" search function thing, so it
//knows to search only in that one? And they all already search in the "named" section. ok. What knows to search? First, we search in a single var list
//for a name/value. Then, we can use a way to do that multiple times in a larger function. Ok.

//And we have nested variable lists...fine. We can literally add it to the structures?

struct functlist_item
{
  std::string tag;
  std::function<std::string(functsig)> funct; //args.size() is the CORRECT number of args? No, we need to know the "correct" number of args. AH, this is the
  // SIGNATURE of the function to call, i.e. the function is just a funct that takes a functsig. Here when I "register" it, I need to tell it the correct
  //nargs. So, the check will happen here.
  size_t nargs;
  
  
  //Constructor
  functlist_item( const std::string _tag, const std::function<void(functsig)> _funct, const size_t _nargs )
  : tag(_tag),
    funct(_funct),
    nargs(_nargs)
  {
    //REV: do nothing
  }
  
  std::string execute( const functsig& fs )
  {
    if( fs.args.size() != nargs )
      {
	fprintf(stderr, "ERROR: in execute of functlist_item [%s]: nargs (%ld) != args of passed functsig (%ld)\n", tag.c_str(), nargs, fs.args.size());
	exit(1);
      }
    else
      {
	return funct( fs );
      }
  }
};
