#pragma once

#include <variable.h>
#include <memfsys.h>

template <typename T>
struct hierarchical_varlist
{
  std::vector< varlist<T> > vl;

  std::vector< std::vector< size_t > > children;
  std::vector< size_t > parents; //who is the parent of root "0"? There is none.
  
  const size_t get_parent( const size_t& v );


  void tofile( const std::string& fname, const size_t& starti, memfsys& myfsys, const bool& usedisk=false);
  
  const std::vector<size_t> get_children( const size_t& v );
  
  size_t add_child( size_t parent, const varlist<T>& v );

  void add_root( const varlist<T>& v );

  //Depth can actually be inferred by checking # jumps to root.
  void recursively_enumerate(const size_t& node_idx, const size_t& depth); //need depth? For number of tabs?
    
  void enumerate();
  
  hierarchical_varlist();

  hierarchical_varlist(const varlist<T>& start_root);
  

  
  size_t find_var_in_hierarchy( const std::string& varname, const size_t& startvl_idx, std::vector< size_t >& vl_indices, std::vector< size_t >& v_indices );
  


  //Only works if var is ARRAY type.
  void add_to_var( const std::string& targ, const T _v, const size_t startvl );
  
  void add_array_to_var( const std::string& targ, const std::vector<T>& _v, const size_t startvl );
  
  void setvar( const std::string& targ, variable<T>& _v, const size_t startvl );
  
  void addvar( const std::string& targ, variable<T>& _v, const size_t startvl );
  
  variable<T> getvar( const std::string& targ, const size_t startvl );
 

  std::vector<T> get_array_var( const std::string& targ, const size_t startvl );

  T get_val_var( const std::string& targ, const size_t startvl );
  
  //Search for a target variable in a hierarchy (given a source "leaf" or any pointer inside here? Making it way too complex?)
  
}; //end hierarchical varlist
