#include <variable.h>

template <typename T>
struct hierarchical_varlist
{
  std::vector< varlist<T> > vl;

  std::vector< std::vector< size_t > > children;
  std::vector< size_t > parents; //who is the parent of root "0"? There is none.
  
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
  
  void add_child( size_t parent, const varlist<T>& v )
  {
    if( parent >= vl.size() )
      {
	fprintf(stderr, "Error trying to add to a parent that doesn't exist...use add_root()\n");
      }
    vl.push_back( v );
    size_t myindex = vl.size() - 1 ;
    children[ parent ].push_back( myindex );
    children.push_back( std::vector<size_t>(0) );
    parents.push_back(parent);
  }

  void add_root( const varlist<T>& v )
  {
    vl.push_back( v );
    children.push_back( std::vector<size_t>(0) );
  }


  //Depth can actually be inferred by checking # jumps to root.
  void recursively_enumerate(size_t node_idx, size_t depth) //need depth? For number of tabs?
  {
    if(node_idx >= vl.size() )
      {
	fprintf(stderr, "ERROR in recursively enum: node_idx is outside of vl array size (%ld)\n", node_idx );
	exit(1);
      }
    
    vl[node_idx].enumerate( depth );
    for(size_t c = 0; c < get_children(node_idx).size(); ++c)
      {
	recursively_enumerate( get_children(node_idx)[c], depth+1 );
      }

    return;
  }
  
  void enumerate()
  {
    //start with root.
    size_t depth=0;

    //Recursively enumerate children.
    recursively_enumerate( 0, 0 ); //root, zero depth.
  }

  
  hierarchical_varlist()
  {
    fprintf(stderr, "REV: this is messed, shouldn't use empty constructor b/c we lose track of root. Exiting\n");
    exit(1);
    //There is nothing... must use constructor with root.
  }

  hierarchical_varlist(const varlist<T>& start_root)
  {
    vl.push_back( start_root ); //no need to push back parent, there are none. Push back children I guess.
    children.push_back( std::vector<size_t>(0) );
  }
  

  
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


  variable<T> getvar( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, vl_indices, v_indices );

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
	return ( vl[ vl_indices[0] ].getvar( targ ) );
      }
  }

  std::vector<T> get_array_var( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, vl_indices, v_indices );

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
	return (vl[ vl_indices[0] ].getArrayvar( targ ) );
      }
    
  }

  T get_val_var( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, vl_indices , v_indices );

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
	return ( vl[ vl_indices[0] ].getTvar( targ ) );
      }
    
  }
  //Search for a target variable in a hierarchy (given a source "leaf" or any pointer inside here? Making it way too complex?)
  
}; //end hierarchical varlist
