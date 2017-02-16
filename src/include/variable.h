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

#include <commontypes.h>

#include <new> //need this for variant in construcor type things? Maybe not, was doing it for unions with non-POD

//#include <memfile.h>

#include <memfsys.h>
//REV: includes...

//REV: well I fucked up. I can't use union. Need to use boost variant, or do my own thing with inhereitance and pointers.


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
  inline void serialize(Archive & ar, const unsigned int version);
  
  
  inline std::vector< T >
  get_v( );
  
  inline T
  get_s( );
  

  inline variable<T>( const std::string _name, const T& _val);
  

  inline variable<T>();
  
  
  inline variable<T>( const std::string _name, const std::vector<T>& _val);
 

  inline variable<T>( const std::string& n, const variable<T>& _v);
 

  
  //REV: this won't work unless T is string haha...
  //REV: THIS WILL BREAK UNLESS TYPE IS STD::STRING!!!!
  //Note if you try to compile and get an error here!
  inline std::string asstring();
 
  inline std::string gettype_asstr();
 
    
};


template <typename T>
struct varlist
{
  std::string mytag;
  std::vector< variable<T> > vars;


  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  inline void serialize(Archive & ar, const unsigned int version);
 
  
  
  //append? or overwrite? Default is APPEND to end.
  inline void tofile( const std::string& fname, memfsys& mf, const bool& usefile=false );
 
  
  //append? or overwrite? Default is APPEND to end.
  inline void tofile( const std::string& fname );
  


  inline void inputfromfile( const std::string& fname, memfsys& mf, const bool& readthrough=false );
  
  
  inline void inputfromfile( const std::string& fname );
  

  inline void mergevarlist( const varlist& v );
  

  inline void enumerate(size_t depth=0);
  
  
  
  inline varlist();
  
  

  inline varlist( const std::string& _tag );
  
  template <typename TT>
  inline void add_to_varlist( const std::vector<std::string>& varnames, const std::vector<TT>& varvals );
  

  //modifies it if it exists, otherwise adds it.
  inline void setvar( const std::string& _varname, const variable<T>& _v );
  

 inline  void addvar( const variable<T>& _v );
 
  
  inline void addvar( const std::string& _varname, const variable<T>& _v );
  
  inline void addTvar( const std::string& _varname, const T& _v);
  
  inline void addArrayvar( const std::string& _varname, const std::vector<T>& _v);
  

  inline variable<T> getvar( const std::string& _varname );
 

 inline  T getTvar( const std::string& _varname );
  

 inline  std::vector<T> getArrayvar( const std::string& _varname );
  
  inline std::vector<size_t> getname(const std::string& name);
 

  template <typename TT>
 inline  void make_varlist( const std::vector<std::string>& vnames, const std::vector<TT>& vvals );
 
 inline  void add_float64( const std::string& v, const float64_t& val );
 

  inline void add_int64( const std::string& v, const int64_t& val );
  
  
 inline  float64_t get_float64( const std::string& v );
  

 inline  int64_t get_int64(const std::string& v);
  
  
};
  
#include <variable.cpp>
