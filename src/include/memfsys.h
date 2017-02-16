#pragma once

#include <memfile.h>
#include <utility_functs.h>

//struct mem_filesys
struct memfsys
{
  std::vector< memfile > filelist;
  
  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  inline void serialize(Archive & ar, const unsigned int version);
  


  inline void enumerate();
 
  inline bool check_existence( const std::string& fname, const bool& checkdisk=false );
 
  inline std::vector<size_t> find_string_in_memfile_vect( const std::string& targ );
 
  inline void add_file( const memfile& m );
 
  
  inline memfile_ptr open( const std::string& fname, const bool& readthrough=false );
 
  
  inline memfile_ptr open_from_disk( const std::string& fname );
 
  
  inline memfile get_memfile( const std::string& fname );
  

  
};

#include <memfsys.cpp>
