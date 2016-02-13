#pragma once

#include <memfile3.h>


//struct mem_filesys
struct memfsys
{
  std::vector< memfile > filelist;

  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & filelist;
  }


  std::vector<size_t> find_string_in_memfile_vect( const std::string& targ )
  {
    std::vector<size_t> ret;
    for(size_t x=0; x<filelist.size(); ++x)
      {
	if( filelist[x].filename.compare( targ ) == 0 )
	  {
	    ret.push_back( x);
	  }
      }
    return ret;
  }

  void add_file( const memfile& m )
  {
    filelist.push_back( m );
  }
  
  memfile_ptr open( const std::string& fname, const bool& readthrough=false )
  {
    std::vector<size_t> locs = find_string_in_memfile_vect( fname );
    if( locs.size() == 0 )
      {
	memfile mf( fname, readthrough );
	filelist.push_back( mf );
	return memfile_ptr( filelist[ filelist.size()-1 ] );
      }
    else if( locs.size() == 1 ) //REV: Huh, doesn't bother reading through if it already exists? This seems bad???
      {
	return memfile_ptr( filelist[ locs[0] ] );
      }
    else
      {
	fprintf(stderr, "ERROR, more than one file of given name [%s] found!\n", fname.c_str());
	exit(1);
      }
  }

  memfile_ptr open_from_disk( const std::string& fname )
  {
    return open( fname, true );
  }
  
  memfile get_memfile( const std::string& fname )
  {
    std::vector<size_t> locs = find_string_in_memfile_vect( fname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR, getting memfile, either not found or more than one! [%s], [%ld] copies\n", fname.c_str(), locs.size() );
	exit(1);
      }

    return filelist[ locs[0] ];
  }

  
};
