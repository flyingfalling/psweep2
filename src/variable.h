#pragma once

//Crap, need to use variant after all orz
#include <boost/variant.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

#include <new>

//REV: includes...

//REV: well I fucked up. I can't use union. Need to use boost variant, or do my own thing with inhereitance and pointers.


//make this a base variable, and everything is derived from this type...?
template <typename T>
struct variable
{
  enum type {  VAR,  ARRAY  };
  
  type mytype;
  std::string name;

  boost::variant< T, std::vector<T> > val;
  
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

  variable<T>( const std::string _name, const std::vector<T>& _val)
  {
    //it's a variable...
    val = _val;
    name = _name;
    mytype = ARRAY;
  }
    
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

template <typename T>
struct varlist
{
  std::string mytag;
  std::vector< variable<T> > vars;


  void enumerate()
  {
    fprintf(stdout, "ENUMERATING varlist [%20s]\n", mytag.c_str() );
    for(size_t v=0; v<vars.size(); ++v)
      {
	std::string asstr = vars[v].asstring();
	fprintf(stdout, "VAR [%5ld] NAME: (%20s): TYPE (%5s):  VALUE: [%s]\n", v, vars[v].name.c_str(), vars[v].gettype_asstr().c_str(), asstr.c_str());
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
  
  void addvar( const std::string& _varname, const variable<T>& _v )
  {
    vars.push_back (_v);
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
	if( name == vars[x].name )
	  {
	    rlocs.push_back(x);
	  }
      }
    return rlocs;
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
