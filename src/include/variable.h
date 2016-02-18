#pragma once

//Crap, need to use variant after all orz
#include <boost/variant.hpp>

#include <boost/serialization/variant.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

#include <utility_functs.h>

#include <new> //need this for variant in construcor type things? Maybe not, was doing it for unions with non-POD

#include <memfile3.h>

#include <memfsys.h>
//REV: includes...

//REV: well I fucked up. I can't use union. Need to use boost variant, or do my own thing with inhereitance and pointers.

typedef double float64_t;
typedef long int int64_t;

//make this a base variable, and everything is derived from this type...?
template <typename T>
struct variable
{
  enum type {  VAR,  ARRAY  };
  
  type mytype;
  std::string name;
  

  //REV: OK MAIN PROBLEM IS THAT I CANNOT SERIALIZE VARIANTS CONTAINING ARBITRARY DATA TYPES?!
  boost::variant< T, std::vector<T> > val;

  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & mytype;
    ar & name;
    ar & val;
  }
  
  std::vector< T >
  get_v( )
  {
    return boost::get<std::vector<T>>( val );
  }
  
  T
  get_s( )
  {
    return boost::get<T>( val );
  }

  variable<T>( const std::string _name, const T& _val)
  {
    //it's a variable...
    val = _val;
    name = _name;
    mytype = VAR;
  }

  variable<T>()
  {
    name="ERRORNOVARNAME";
  }
  
  variable<T>( const std::string _name, const std::vector<T>& _val)
  {
    //it's a variable...
    val = _val;
    name = _name;
    mytype = ARRAY;
  }

  variable<T>( const std::string& n, const variable<T>& _v)
  {
    val = _v.val;
    name = n;
    mytype = _v.mytype;
  }

  
  //REV: this won't work unless T is string haha...
  //REV: THIS WILL BREAK UNLESS TYPE IS STD::STRING!!!!
  //Note if you try to compile and get an error here!
  std::string asstring()
  {
    if( mytype == VAR )
      {
	return get_s(); //std::to_string( s_val );
      }
    else if (mytype == ARRAY )
      {
	std::string toret="";
	std::vector<T> vval=get_v();
	for(size_t a=0; a<vval.size(); ++a)
	  {
	    toret += " " + vval[a]; //std::to_string( v_val[ a ]);
	  }
	return toret;
      }
  }

  std::string gettype_asstr()
  {
     if( mytype == VAR )
      {
	return "VAR";
      }
    else if (mytype == ARRAY )
      {
	return "ARRAY";
      }
     return "ERROR";
  }
    
};


#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

template <typename T>
struct varlist
{
  std::string mytag;
  std::vector< variable<T> > vars;


  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & mytag;
    ar & vars;
  }
  
  
  //append? or overwrite? Default is APPEND to end.
  void tofile( const std::string& fname, memfsys& mf, const bool& usefile=false )
  {
    //std::ofstream f;
    //open_ofstream( fname, f );

    //memfile_ptr mfp = mf.get_ptr( fname );
    //Doesn't matter at first, we're making a new one anyways...truncating? Yes. Not "append to file" or some shit.
    //REV: There is no writethrough! I need to APPEND!!! FUCKKKKKKKK (same with FROMFILE)
    //fprintf(stdout, "Opening file for output (var): [%s]\n", fname.c_str());
    memfile_ptr mfp = mf.open( fname, false );
    
    //Only prints out vars if they are strings, not if they are arrays.
    //Should allow to print out arrays too? What types of things might
    //be arrays?
    
    for(size_t v=0; v<vars.size(); ++v)
      {
	//fprintf(stdout, "Attempting to print out to file [%ld]th var: %s %s\n", v, vars[v].name.c_str(), vars[v].get_s().c_str() );
	//REV: It's trying to write CMD array variable to the file which is fucked up. I don't want that. I only want the root parameters I think? Crap.
	//OVERLOAD variable so that it appropriately outputs it if it is an array.
	if( vars[v].mytype == variable::type::VAR )
	  {
	    mfp << vars[v].name << " " << vars[v].get_s() << std::endl;
	  }
	
	//f << vars[v].name << " " << vars[v].get_s() << std::endl;
	//mfp.printf( "%s %s\n", vars[v].name.c_str(), vars[v].get_s().c_str() );
	
      }

    
    if( usefile == true )
      {
	fprintf(stdout, "Attempting to print out to actual files...\n");
	//print to the disk for real...
	mfp.tofile( fname );
      }
    
    mfp.close();
    
    //close_ofstream( f );

    return;
  }
  
  //append? or overwrite? Default is APPEND to end.
  void tofile( const std::string& fname )
  {
    std::ofstream f;
    open_ofstream( fname, f );
    //Only prints out vars if they are strings, not if they are arrays.
    //Should allow to print out arrays too? What types of things might
    //be arrays?
    for(size_t v=0; v<vars.size(); ++v)
      {
	//REV: It's trying to write CMD array variable to the file which is fucked up. I don't want that. I only want the root parameters I think? Crap.
	//OVERLOAD variable so that it appropriately outputs it if it is an array.
	if( vars[v].mytype == variable::type::VAR )
	  {
	    //f << vars[v].name << " " << vars[v].get_s() << std::endl;
	    f << vars[v].name << " " << vars[v].get_s() << std::endl;
	  }
      }
    
    close_ofstream( f );

    return;
  }


  void inputfromfile( const std::string& fname, memfsys& mf, const bool& readthrough=false )
  {
    //No read through...to actual FS. I.e. everything done through memory.
    //This is really a problem, I need to organize this better. I.e.
    //tell switch of MEM_FILESYSTEM to turn on or off reading/writing to
    //actual files...
    //fprintf(stdout, "In inputfromfile (from mem fsys), opening...\n");
    memfile_ptr mfp = mf.open(fname, readthrough);
    //fprintf(stdout, "In inputfromfile (from mem fsys), OPENED...\n");

    //fprintf(stdout, "INPUTTING FROM FILE: [%s]\n", fname.c_str() );
    
    while( !mfp.eof() )
      {
	std::string n="YOLOERRNAME",
	  v="YOLOERRORVAL";

	//Wow that's ghetto lolol...
	//mfp.scanf( "%s %s\n", n.data(), v.data());
	mfp >> n >> v;
	
	//fprintf(stdout, "Read [%s] for name from file [%s]\n", n.c_str(), fname.c_str() );
	//Check if there is anything at all. If nothing, just empty...


	if( !mfp.bad() )
	  {
	    variable<std::string> var( n, v );
	    addvar( var );
	  }
      }

    mfp.close();


    
    return;
  }
  
  void inputfromfile( const std::string& fname )
  {
    //CAN I READ IN AND THEN PARSE?
    //Or just read in line by line.
    //Wait, shouldn't user-specific program do this reading? I.e.
    //I just pass the filenames or something, and it handles them on
    //a per-algo basis? Or we read everything into a varlist and pass that
    //back?
    
    //Some function that parses into "spaces" or something? Does reading
    //into a string maintain newlines? (Bytes?) I assume it must
    //Just read one at a time.
    std::ifstream f;
    open_ifstream( fname, f );

    //fprintf(stdout, "INPUTTING FROM FILE: [%s]\n", fname.c_str() );

    while( !f.eof() )
      {
	std::string n="YOLOERRNAME", v="YOLOERRORVAL";
	f >> n;

	//fprintf(stdout, "Read [%s] for name from file [%s]\n", n.c_str(), fname.c_str() );
	//Check if there is anything at all. If nothing, just empty...
	if( f.eof() )
	  {
	    break;
	  }
	f >> v;


	//fprintf(stdout, "Read [%s] for val from file [%s]\n", v.c_str(), fname.c_str() );
	//If it was EOF, means it was probably either 1) a new line or 2) a name wihtout a value. In either case ignore.
	//Worst case is name without value followed by a newline...lol.
	if( f.eof() )
	  {
	    break;
	  }
	variable<std::string> var( n, v );
	addvar( var );
	
      }


    
    for( std::string n, v; f >> n >> v; )
      {
	variable<std::string> var( n, v );
	addvar( var );
      }
    
    /*f >> n; //TRIES to read. Will be EOF only if it read EOF
    while( !f.eof() )
      {
	std::string v="YOLOERRORVAL";
	f >> v;
	
	if( f.eof() )
	  {
	    fprintf(stdout, "ERROR IN READ FROM FILE file [%s]: Only got NAME without getting VALUE name: [%s]\n", fname.c_str(), n.c_str());
	    //exit(1);
	    break;
	  }
	
	
	variable<std::string> var( n, v );
	addvar( var );


	
	
	f >> n;
      }
    */

    return;
  }

  void mergevarlist( const varlist& v )
  {
    //for each var in other varlist, add to me. If clashes, fuck it?
    for( size_t var=0; var<v.vl.size(); ++var)
      {
	addvar( v.vl[var] ); //varname is same...
      }
    return;
  }

  void enumerate(size_t depth=0)
  {
    std::string prefix = "";
    for(size_t d=0; d<depth; ++d)
      {
	prefix += " |";
      }
    prefix+="|";
    
    fprintf(stdout, "%sENUMERATING varlist [%20s]\n", prefix.c_str(), mytag.c_str() );
    for(size_t v=0; v<vars.size(); ++v)
      {
	std::string asstr = vars[v].asstring();
	fprintf(stdout, "%sVAR [%5ld] NAME: (%20s): TYPE (%5s):  VALUE: [%s]\n", prefix.c_str(), v, vars[v].name.c_str(), vars[v].gettype_asstr().c_str(), asstr.c_str());
      }
  }
  
  
  varlist()
  : mytag( "ERROR_UNNAMED_VARLIST" )
  {
    //nothing
  }

  varlist( const std::string& _tag )
  :
  mytag (_tag)
  {
    //nothing
  }

  template <typename TT>
  void add_to_varlist( const std::vector<std::string>& varnames, const std::vector<TT>& varvals )
  {
    if(varnames.size() != varvals.size())
      {
	fprintf(stderr, "ERROR add to varlist, VARNAMES != VARVALS SIZE\n"); exit(1);
      }
    for(size_t x=0; x<varnames.size(); ++x)
      {
	//REV: will this even work? What the heck?
	//Will only work if T==std::string
	variable<T> v(varnames[x], std::to_string(varvals[x]) );
	vars.push_back(v);
      }
  }

  //modifies it if it exists, otherwise adds it.
  void setvar( const std::string& _varname, const variable<T>& _v )
  {
    std::vector<size_t> locs = getname( _varname );
    if(locs.size() < 1)
      {
	addvar( _varname, _v );
      }
    else
      {
	if( locs.size() > 1)
	  {
	    fprintf(stderr, "ERROR: ambiguously trying to set a variable that has multiple instances! [%s] (%ld instances)\n", _varname.c_str(), locs.size());
	    exit(1);
	  }
	vars[ locs[0] ] = _v; //just write over the old one. Hopefully it uses the destructor appropriately with all my weird variants etc.
	vars[ locs[0] ].name = _varname;
	//Name should already be == _varname???!!! Ah, but if we set to _v, we will overwrite it! So OK.
	/*if(vars[locs[0]].name != _varname)
	  {
	    fprintf(stderr, "REV: super what the heck? Varname != found var\n"); exit(1);
	    }*/
      }
  }

  void addvar( const variable<T>& _v )
  {
    vars.push_back( _v );
    return;
      
  }
  
  void addvar( const std::string& _varname, const variable<T>& _v )
  {
    //REV: this won't work...need to make sure it doesn't exist?
    
    //REV: this is weird, variables have their own names automatically...i.e. TAG
    variable<T> tmp(_varname, _v);   // = _v;
      //tmp.name = _varname;
    vars.push_back (tmp);
    return;
  }

  void addTvar( const std::string& _varname, const T& _v)
  {
    variable<T> v( _varname, _v );
    vars.push_back( v );
    return;
  }

  void addArrayvar( const std::string& _varname, const std::vector<T>& _v)
  {
    variable<T> v( _varname, _v ); //should construct as array automatically?
    //REV: this won't work automatically unless user knows what to do...
    vars.push_back( v );
    return;
  }

   variable<T> getvar( const std::string& _varname )
  {
    std::vector<size_t> locs = getname( _varname );
    if(locs.size() < 1)
      {
	fprintf(stderr, "ERROR: variables.h: getvar(): did not find name [%s]\n", _varname.c_str());
	exit(1);
      }
    else if(locs.size() > 1)
      {
	fprintf(stdout, "WARNING: Found multiple variables (%ld) with name [%s], will only use first\n", locs.size(), _varname.c_str());
	//Don't exit?
      }

    
    return vars[ locs[0] ];
    
  }

   T getTvar( const std::string& _varname )
  {
    std::vector<size_t> locs = getname( _varname );
    if(locs.size() < 1)
      {
	fprintf(stderr, "ERROR: variables.h: getvar(): did not find name [%s]\n", _varname.c_str());
	exit(1);
      }
    else if(locs.size() > 1)
      {
	fprintf(stdout, "WARNING: Found multiple variables (%ld) with name [%s], will only use first\n", locs.size(), _varname.c_str());
	//Don't exit?]
      }
    
    return vars[ locs[0] ].get_s();
  }

   std::vector<T> getArrayvar( const std::string& _varname )
  {
    std::vector<size_t> locs = getname( _varname );
    if(locs.size() < 1)
      {
	fprintf(stderr, "ERROR: variables.h: getvar(): did not find name [%s]\n", _varname.c_str());
	exit(1);
      }
    else if(locs.size() > 1)
      {
	fprintf(stdout, "WARNING: Found multiple variables (%ld) with name [%s], will only use first\n", locs.size(), _varname.c_str());
	//Don't exit?
      }
    
    return vars[ locs[0] ].get_v();
  }

  std::vector<size_t> getname(const std::string& name)
  {
    std::vector<size_t> rlocs;
    for(size_t x=0; x<vars.size(); ++x)
      {
	if( name.compare( vars[x].name ) == 0 )
	  {
	    rlocs.push_back(x);
	  }
      }
    return rlocs;
  }

  template <typename TT>
  void make_varlist( const std::vector<std::string>& vnames, const std::vector<TT>& vvals )
  {
    if(vnames.size() != vvals.size() )
      {
	fprintf(stderr, "Error in make varlist vvals != vnames size\n");
	exit(1);
      }
    for(size_t x=0; x<vnames.size(); ++x)
      {
	std::stringstream _ss("");
	_ss << vvals[x];
	addTvar( vnames[x], _ss.str() );
      }
    return;
  }

  float64_t get_float64(const std::string& v)
  {
    std::string val = getTvar( v );
    return std::stof( val );
  }

  int64_t get_int64(const std::string& v)
  {
    std::string val = getTvar( v );
    return std::stol( val );
  }
  
};
  

//#include <variable.tpp>


/*
struct varlist
{
  enum vartype
  {
    VAR,
    ARRAY
  };

  std::string mytag; //allow multiple tags?
  
  std::vector< std::string > names;
  std::vector< vartype > tags;
  std::vector< size_t > indices; //which index in array or var place it is.

  std::vector< std::string > varvals;
  std::vector< std::vector< std::string > > arrayvals;

  
  //make it fail if I double up? Or make it array?
  void addvar(const std::string& name, const std::string& val)
  {
    //Check if it exists first?
    names.push_back(name);
    tags.push_back( VAR );
    indices.push_back( varvals.size() ); //I.e. index BEFORE push-back. So, at 0 length, index of new guy will be 0 (i.e. first element)
    varvals.push_back(val);
  }

  void addarray(const std::string& name, const std::vector<std::string>& val)
  {
    //Check if it exists first?
    names.push_back(name);
    tags.push_back( ARRAY ); //REV: NEED TO KNOW WHICH INDEX IT IS IN THIS THING! SO  NEED SOME SIMPLER CLASS THAT CONTAINS A TAGGED INDEX
    indices.push_back( arrayvals.size() ); //I.e. index BEFORE push-back. So, at 0 length, index of new guy will be 0 (i.e. first element)
    arrayvals.push_back(val);
  }

  std::vector<size_t> getname(const std::string& name)
  {
    std::vector<size_t> rlocs;
    for(size_t x=0; x<names.size(); ++x)
      {
	if( name == names[x] )
	  {
	    rlocs.push_back(x);
	  }
      }
    return rlocs;
  }
  
  //get as array first finds one with names, then grabs first one of those?
  std::vector< std::string > getarray(const std::string& name)
  {
    std::vector<size_t> locs = getname( name );
    if(locs.size() < 1)
      {
	fprintf(stderr, "ERROR: variables.h: getarray(): did not find name [%s]\n", name.c_str());
	exit(1);
      }
    else if(locs.size() > 1)
      {
	fprintf(stdout, "WARNING: Found multiple variables (%ld) with name [%s], will only use first\n", locs.size(), name.c_str());
	//Don't exit?
      }
    else
      {
	if(tags[ locs[0] ] != ARRAY)
	  {
	    fprintf(stderr, "ERROR, returned type is not of ARRAY (getarray())\n");
	    exit(1);
	  }
	return arrayvals[ indices[ locs[0] ] ];
      }
  }

  std::string getvar(const std::string& name)
  {
    std::vector<size_t> locs = getname( name );
    if(locs.size() < 1)
      {
	fprintf(stderr, "ERROR: variables.h: getvar(): did not find name [%s]\n", name.c_str());
	exit(1);
      }
    else if(locs.size() > 1)
      {
	fprintf(stdout, "WARNING: Found multiple variables (%ld) with name [%s], will only use first\n", locs.size(), name.c_str());
	//Don't exit?
      }
    else
      {
	if(tags[ locs[0] ] != VAR)
	  {
	    fprintf(stderr, "ERROR, returned type is not of ARRAY (getarray())\n");
	    exit(1);
	  }
	return varvals[ indices[ locs[0] ] ];
      }
  }

  //Function to "make array". Functions to "get arrays" in certain loop ways (return construct strings).
  //Function to specify main argument. Function to add additional args. Those are "pset" specific things though, not "variable" things!
  //How about hierarchy thing? Meh...need a way to "refer" to pset things? Make a varlist for each one I guess? With some way of parsing it from
  //user? Name of varset for example...?
  
};
*/
