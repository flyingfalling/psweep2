
//REV: includes...



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
