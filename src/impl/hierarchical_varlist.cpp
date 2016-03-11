
#pragma once
#include <hierarchical_varlist.h>

template <typename T>
const size_t hierarchical_varlist<T>::get_parent( const size_t& v )
  {
    if(v == 0)
      {
	fprintf(stderr, "ERROR in get_parent, trying to get parent of root...\n");
	exit(1);
      }
    //REV: Because user will pass "actual" v he wants, not the one that doesnt include the root as 0, which is what we are thinking of...
    else if( v <= parents.size() ) //bc root has no parent...
      {
	return parents[v-1]; //pain in the butt.
      }
    else
      {
	fprintf(stderr, "ERROR IN get_parent: tried to get [%ld]-th member of parent vector, but its out of bounds (vector is [%ld]-length)\n", v, parents.size());
	exit(1);
      }
  }

template <typename T>
void hierarchical_varlist<T>::tofile( const std::string& fname, const size_t& starti, memfsys& myfsys, const bool& usedisk)
  {
    if(starti==0)
      {
	//it is root.
	vl[starti].tofile( fname, myfsys, usedisk );
      }
    else
      {
	size_t idx=starti;
	while( idx > 0 )
	  {
	    //fprintf(stdout, "WRITING TO FILE IDX=%ld\n", idx);
	    vl[idx].tofile( fname, myfsys, usedisk ); //wow this will open and close it again, definitely not worth it. It's appending recall. OK.
	    //fprintf(stdout, "FINISHED WRITING\n");
	    idx = get_parent( idx );
	    //fprintf(stdout, "GOT PARENT AFTER WRITING...\n");
	  }
	if( idx != 0)
	  {
	    fprintf(stderr, "BIG ERROR, failed to end up at root while walking through hierarchical varlist\n");
	    exit(1);
	  }
	vl[idx].tofile( fname, myfsys, usedisk ); //finally, this should be root.
	
      }
    
  } //end tofile.
template <typename T>
const std::vector<size_t> hierarchical_varlist<T>::get_children( const size_t& v )
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

template <typename T>
  size_t hierarchical_varlist<T>::add_child( size_t parent, const varlist<T>& v )
  {
    if( parent >= vl.size() )
      {
	fprintf(stderr, "Error trying to add to a parent that doesn't exist...use add_root()\n");
      }
    vl.push_back( v );
    size_t myindex = vl.size() - 1 ;
    children[ parent ].push_back( myindex );
    children.push_back( std::vector<size_t>(0) );
    parents.push_back( parent );
    return (vl.size()-1); //returns index of it.
  }

template <typename T>
void hierarchical_varlist<T>::add_root( const varlist<T>& v )
  {
    vl.push_back( v );
    children.push_back( std::vector<size_t>(0) );
  }

template <typename T>
  //Depth can actually be inferred by checking # jumps to root.
void hierarchical_varlist<T>::recursively_enumerate(const size_t& node_idx, const size_t& depth) //need depth? For number of tabs?
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

  template <typename T>
void hierarchical_varlist<T>::enumerate()
  {
    //start with root.
    size_t depth=0;

    //Recursively enumerate children.
    recursively_enumerate( 0, 0 ); //root, zero depth.
  }


template <typename T>
hierarchical_varlist<T>::hierarchical_varlist()
  {
    fprintf(stderr, "REV: this is messed, shouldn't use empty constructor b/c we lose track of root. Exiting\n");
    exit(1);
    //There is nothing... must use constructor with root.
  }

template <typename T>
hierarchical_varlist<T>::hierarchical_varlist(const varlist<T>& start_root)
  {
    vl.push_back( start_root ); //no need to push back parent, there are none. Push back children I guess.
    children.push_back( std::vector<size_t>(0) );
  }
  

template <typename T>
size_t hierarchical_varlist<T>::find_var_in_hierarchy( const std::string& varname, const size_t& startvl_idx, std::vector< size_t >& vl_indices, std::vector< size_t >& v_indices )
  {
    size_t searchi=startvl_idx;
    //While not root.
    while ( searchi != 0 )
      {
	//Search THIS varlist for the target name. 
	//  namelocs[] holds the local indices of the found ones.
	//I push back to vl_indices each one, etc.
	std::vector<size_t> namelocs = vl[searchi].getname( varname );
	for( size_t x=0; x<namelocs.size(); ++x )
	  {
	    vl_indices.push_back(searchi);
	    v_indices.push_back(namelocs[x]);
	  }

	//size_t tm=searchi;
	
	searchi = get_parent( searchi );
	//fprintf(stdout, "Got parent! (%ld to %ld)\n", tm, searchi );
	//fprintf(stdout, "Got parent of %ld: it is: %ld\n", searchi, get_parent(searchi));
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


  //Only works if var is ARRAY type.
template <typename T>
void hierarchical_varlist<T>::add_to_var( const std::string& targ, const T _v, const size_t startvl )
  {
    //fprintf(stdout, "In add to var...trying to get array var\n");
    std::vector<T> r = get_array_var( targ, startvl );
    //fprintf(stdout, "DONE got array var...\n");
    r.push_back( _v );
    variable<T> tmp( targ, r ); //"name" is targ, note that setvar will automatically set that anyways...
    //fprintf(stdout, "Will try to set var...\n");
    setvar( targ, tmp, startvl );
    //fprintf(stdout, "DONE set var...returning.\n");
    return;
  }//Only works if var is ARRAY type.

template <typename T>
void hierarchical_varlist<T>::add_array_to_var( const std::string& targ, const std::vector<T>& _v, const size_t startvl )
  {
    //fprintf(stdout, "In add to var...trying to get array var\n");
    std::vector<T> r = get_array_var( targ, startvl );
    //fprintf(stdout, "DONE got array var...\n");
    for(size_t x=0; x<_v.size(); ++x)
      {
	r.push_back( _v[x] );
      }
    
    variable<T> tmp( targ, r ); //"name" is targ, note that setvar will automatically set that anyways...
    //fprintf(stdout, "Will try to set var...\n");
    setvar( targ, tmp, startvl );
    //fprintf(stdout, "DONE set var...returning.\n");
    return;
  }

  template <typename T>
  void hierarchical_varlist<T>::setvar( const std::string& targ, variable<T>& _v, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, vl_indices, v_indices );
    
    if( nfound > 1 )
      {
	//fprintf(stderr, "REV: WARNING: get_array_var in hierarchical (while trying to set var): more than one instance found! (Var=[%s]). Will set leaf-most by default\n", targ.c_str());
	//enumerate();
	//exit(1);
	//REV: what the heck? This should still grab the LEAF-most node!!!
	vl[ vl_indices[0] ].setvar( targ, _v );
      }
    else if( nfound == 0 )
      {
	//fprintf(stdout, "About to try to report adding new var\n");
	//fprintf(stdout, "Adding new variable [%s] [(name=%s) (val=%s)]\n", targ.c_str(), _v.name.c_str(), _v.get_s().c_str());
	//just do an "addvar"
	addvar( targ, _v, startvl );
      }
    else
      {
	//fprintf(stdout, "About to try to report setting existing var\n");
#if DEBUGLEVEL>5
	fprintf(stdout, "Setting existing variable [%s] [(name=%s)]]\n", targ.c_str(), _v.name.c_str() ); //REV: haha retard you were trying to print get_s in a case where it might have been an array.
#endif
	vl[ vl_indices[0] ].setvar( targ, _v );
      }
  }

template <typename T>
void hierarchical_varlist<T>::addvar( const std::string& targ, variable<T>& _v, const size_t startvl )
  {
    //REV: this IGNORES any existing guy!
    vl[ startvl ].addvar( targ, _v );
    return;
  }


template <typename T>
variable<T> hierarchical_varlist<T>::getvar( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, vl_indices, v_indices );

    //fprintf(stdout, "IN GET VAR!!! Searching for [%s]\n", targ.c_str());
    
    if( nfound > 1 )
      {
	//fprintf(stderr, "REV: WARNING: get_array_var in hierarchical: more than one instance found! (Var=[%s]). Will set leaf-most by default.\n", targ.c_str());
	return ( vl[ vl_indices[0] ].getvar( targ ) );
	//enumerate();
	//exit(1);
      }
    else if( nfound == 0 )
      {
	fprintf(stderr, "REV: ERROR: get_array_var in hierarchical: no instances of target var found! (Var=[%s])\n", targ.c_str() );
	enumerate();
	exit(1);
      }
    else
      {
	//wasteful, this is re-searching...
	return ( vl[ vl_indices[0] ].getvar( targ ) );
      }
  }

template <typename T>
std::vector<T> hierarchical_varlist<T>::get_array_var( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;

    //fprintf(stdout, "In array var, about to find var in hierarchy, starting from [%ld]\n", startvl);
    size_t nfound = find_var_in_hierarchy( targ, startvl, vl_indices, v_indices );

    //fprintf(stdout, "DONE found in hierarchy\n");
    if( nfound > 1 )
      {
	fprintf(stderr, "REV: WARNING: get_array_var in hierarchical: more than one instance found! (Var=[%s]). Will return leaf-most by default.\n", targ.c_str());
	enumerate();
	return (vl[ vl_indices[0] ].getArrayvar( targ ) );
	
	//exit(1);
      }
    else if( nfound == 0 )
      {
	fprintf(stderr, "REV: ERROR: get_array_var in hierarchical: no instances of target var found! (Var=[%s])\n", targ.c_str() );
	enumerate();
	exit(1);
      }
    else
      {
	
	//wasteful, this is re-searching...
	return (vl[ vl_indices[0] ].getArrayvar( targ ) );
      }
    
  }

template <typename T>
T hierarchical_varlist<T>::get_val_var( const std::string& targ, const size_t startvl )
  {
    std::vector< size_t > vl_indices;
    std::vector< size_t > v_indices;
    size_t nfound = find_var_in_hierarchy( targ, startvl, vl_indices, v_indices );

    if( nfound > 1 )
      {
	//fprintf(stderr, "REV: WARNING: get_val_var in hierarchical: more than one instance found! (Var=[%s]). Will return leaf-most by default\n", targ.c_str());
	//REV: Return the MOST CHILD-MOST.
	return ( vl[ vl_indices[0] ].getTvar( targ ) );
	//enumerate();
	//exit(1);
      }
    else if( nfound == 0 )
      {
	fprintf(stderr, "REV: ERROR: get_val_var in hierarchical: no instances of target var found! (Var=[%s])\n", targ.c_str() );
	enumerate();
	exit(1);
      }
    else
      {
	//wasteful, this is re-searching...
	return ( vl[ vl_indices[0] ].getTvar( targ ) );
      }
    
  }
  
