#pragma once

#include <variable.h>
#include <memfsys.h>

template <typename T>
struct hierarchical_varlist
{
  std::vector< varlist<T> > vl;

  std::vector< std::vector< size_t > > children;
  std::vector< size_t > parents; //who is the parent of root "0"? There is none.
  
  inline const size_t get_parent( const size_t& v );


  inline void tofile( const std::string& fname, const size_t& starti, memfsys& myfsys, const bool& usedisk=false);
  
  inline const std::vector<size_t> get_children( const size_t& v );
  
  inline size_t add_child( size_t parent, const varlist<T>& v );

  inline void add_root( const varlist<T>& v );

  //Depth can actually be inferred by checking # jumps to root.
  inline void recursively_enumerate(const size_t& node_idx, const size_t& depth); //need depth? For number of tabs?
    
  inline void enumerate();
  
  inline hierarchical_varlist();

  inline hierarchical_varlist(const varlist<T>& start_root);
  

  
  inline size_t find_var_in_hierarchy( const std::string& varname, const size_t& startvl_idx, std::vector< size_t >& vl_indices, std::vector< size_t >& v_indices );
  


  //Only works if var is ARRAY type.
  inline void add_to_var( const std::string& targ, const T _v, const size_t startvl );
  
  inline void add_array_to_var( const std::string& targ, const std::vector<T>& _v, const size_t startvl );
  
  inline void setvar( const std::string& targ, variable<T>& _v, const size_t startvl );
  
  inline  void addvar( const std::string& targ, variable<T>& _v, const size_t startvl );
  
  inline variable<T> getvar( const std::string& targ, const size_t startvl );
 

 inline  std::vector<T> get_array_var( const std::string& targ, const size_t startvl );

 inline  T get_val_var( const std::string& targ, const size_t startvl );
  
  //Search for a target variable in a hierarchy (given a source "leaf" or any pointer inside here? Making it way too complex?)
  
}; //end hierarchical varlist


#include <hierarchical_varlist.cpp>
