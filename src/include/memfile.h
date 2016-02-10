#pragma once


#include <cstdio>
#include <string>
#include <sstream>

#include <boost/mpi.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/filesystem.hpp>
#include <fstream>

#include <utility_functs.h>
#include <vector>

#include <memory>

//REV: memory file. Holds a file in memory for ghetto way of passing to user program via memory.

//REV: Need to make it archivable, so we can easily do the stuff there.

struct mem_file
{
  std::string filename;
  std::vector< char > filedata;
    
  //Tell it to read as stuff or as binary...
  //Allow user to call sscanf( filedata.data()+offset, "FORMAT", inputs )
  //I.e. I need to wrap it orz.
  
  //Give user way to check EOF? Or he might try to read past the end of vector. Nah.
  //OK, let him read binary stuff too?

  //Give functions to read it as binary data, or as string data.
  //If string data, user can just call sscanf, or we can make an isstringstream.
  
  std::istringstream get_string_stream( const size_t& start_byte_offset, const size_t& end_byte_offset )
  {
    //REV: Haha memcopying.
    std::istringstream _ss( std::string(filedata.begin()+start_byte_offset, filedata.begin()+end_byte_offset) );
    return _ss;
  }

  
  template<typename... Args>
  void printf(const char* fmt, Args... args )
  {

    size_t SPRINTF_BUFF_SIZE=1e3;
    std::vector<char> buffer( SPRINTF_BUFF_SIZE );
    
    
    int written = std::snprintf( buffer.data(), buffer.size(), fmt, args... );
    while( written >= buffer.size() )
      {
	buffer.resize( buffer.size()*2 );
	int written = std::snprintf( buffer.data(), buffer.size(), fmt, args... );
      }
    if( written < 0 )
      {
	//REV: some error
      }
    else
      {
	//we need to push back written characters from buffer to our location.
	buffer.resize( written );
	write( buffer );
      }
  }
  
  std::ostringstream get_out_string_stream( const size_t& start_byte_offset, const size_t& end_byte_offset )
  {
    //REV: Haha memcopying.
    std::ostringstream _ss( std::string(filedata.begin()+start_byte_offset, filedata.begin()+end_byte_offset) );
    return _ss;
  }

  //REV: Move an internal pointer? No point, I'm not actually consuming anything.
  template <typename T>
  T get_from_binary( const size_t& byte_offset )
  {
    if( (byte_offset + sizeof( T )) > filedata.size() )
      {
	fprintf(stderr, "ERROR, trying to get from offset [%ld], a type, which exceeds size of array [%ld]\n", byte_offset, filedata.size());
	exit(1);
      }
    return *((T*)(filedata.data() + byte_offset));
  }

  template <typename T>
  std::vector<T> get_array_from_binary( const size_t& byte_offset, const size_t& num_items )
  {
    if( (byte_offset + (num_items*sizeof( T ))) > filedata.size() )
      {
	fprintf(stderr, "ERROR, trying to get ARRAY from offset [%ld] plus [%ld], a type, which exceeds size of array [%ld]\n", byte_offset, (num_items*sizeof( T )), filedata.size());
	exit(1);
      }
    std::vector<T> ret(num_items);
    std::copy( ret.data(), ret.data() + (num_items * sizeof(T)), filedata.data()+byte_offset );
    return ret;
  }
  
  mem_file( const std::string& memfname, const std::vector<char>& content )
  {
    filedata=content;
    filename = memfname;
  }


  //REV: this is an INFILE!!!
  mem_file( const std::string& fname, bool append=true )
  {
    // open the file:
    std::streampos fileSize;
    std::fstream file;
    if(append)
      {
	file = std::fstream(fname, std::fstream::in | std::ios::binary | std::fstream::out | std::fstream::app);
      }
    else
      {
	file = std::fstream(fname, std::fstream::in | std::ios::binary | std::fstream::out | std::fstream::trunc);
      }
    // get its size:
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // read the data:
    //std::vector<char> fileData(fileSize);
    filedata.resize(fileSize);
    file.read(filedata.data(), fileSize);

    filename = fname;
  }
  
  void write( const std::vector<char>& towrite, const bool append=true )
  {
    if(append)
      {
	size_t offset=filedata.size();
	filedata.resize( filedata.size() + towrite.size() );
	std::copy( towrite.data(), towrite.data()+towrite.size(), filedata.begin()+offset );
      }
    else
      {
	filedata.resize( towrite.size() );
	std::copy( towrite.data(), towrite.data()+towrite.size(), filedata.begin() );
      }
  }

  void print( const std::string& towrite, const bool append=true )
  {
    if(append)
      {
	size_t offset=filedata.size();
	filedata.resize( filedata.size() + towrite.size() );
	//std::copy( towrite.str(), towrite.data()+towrite.size(), filedata.begin()+offset );
	towrite.copy( filedata.data()+offset, towrite.size(), 0);
      }
    else
      {
	//Either is larger?
	filedata.resize( towrite.size() );
	towrite.copy( filedata.data(), towrite.size(), 0);
	//std::copy( towrite.data(), towrite.data()+towrite.size(), filedata.begin() );
      }
  }

  void tofile(const std::string& dir, const std::string& myfname )
    {
      //REV: this does not use the FNAME of this mem_file...? Ugh.
      std::string fullfname = dir + "/" + myfname;

      std::ofstream ofs;

      open_ofstream( fullfname, ofs );
    
      ofs.write( filedata.data(), filedata.size() );

      //REV: does it work? Need to check sanity before I go?
    }

  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & filename;
    ar & filedata;
  }
};


//Wrap SPRINTF..?
struct memfile_ptr
{
  //Need PTRS to the mem_filesystem so I know where to find the arrays.
  size_t dataptr=0;
  //std::shared_ptr<mem_file> mfile;
  mem_file* mfile;
  
  //If stringstream moves it, I won't know.
  std::istringstream get_string_stream( const size_t& start=0)
  {
    return mfile->get_string_stream( start, mfile->filedata.size() );
  }
  
  template<typename... Args>
  void printf(const char* fmt, Args... args )
  {
    mfile->printf( fmt, args...);
  }
  
  template < typename T >
  std::vector<T> consume_array_from_binary( const size_t& num_items )
  {
    std::vector<T> ret = mfile->get_array_from_binary<T>( dataptr, num_items );
    dataptr += (num_items * sizeof(T) );
    return ret;
  }

  template < typename T >
  T consume_from_binary( )
  {
    T ret = mfile->get_from_binary<T>( dataptr );
    dataptr += sizeof(T);
    return ret;
  }
  
  template < typename T >
  T peek_from_binary( )
  {
    T ret = mfile->get_from_binary<T>( dataptr );
    return ret;
  }

  template < typename T >
  std::vector<T> peek_array_from_binary( const size_t& num_items )
  {
    std::vector<T> ret = mfile->get_array_from_binary<T>( dataptr, num_items );
    return ret;
  }
  //Memfile ptr never "owns" the memory, simply points to it.
  
  //I will actually modify it!!! Ah, don't need a pointer?
  //No...I do. Fuck.
  memfile_ptr( mem_file& mf )
  {
    mfile = &mf; //std::shared_ptr<mem_file>(&mf); //Will this work...? Probably not orz.
    dataptr = 0;
  }

  std::ostringstream get_out_string_stream(const size_t& start=0)
  {
    //REV: something tells me this won't work...it won't write to the underlying char vector?!
    return mfile->get_out_string_stream( start, mfile->filedata.size() );
  }
  
  void print( const std::string& str , bool append=true)
  {
    mfile->print( str );
  }
};


//REV: I don't want to have to try to rebase this...ugh.
//Ohwell. Just give it some "arbitrary" name here haha.
struct mem_filesystem
{
  //Just have linear, there should not be many files.
  //When user writes, he writes to mem_filesystem (first?)
  //When user reads, he reads from mem_filesystem (first?)

  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & filelist;
  }
  
  std::deque< mem_file > filelist;

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



  std::vector<char> read_file( const std::string& fname )
  {
    std::vector<size_t> locs = find_string_in_memfile_vect( fname );
    if( locs.size() == 0 )
      {
	//(attempt to) Open file from actual disk, and read it into std::vector<char>, and return that. In other words, user ONLY uses this
	//to access files -- nothing else.
	//We want to basically redefine ifstream, ofstream, and fstream... and handle all that shit. I.e. overload them. For now just do it my way.
	//will actually make a new mem_file with the name?
	mem_file mf( fname );
	filelist.push_back( mf );
	return (mf.filedata);
      }
    else if( locs.size() == 1 )
      {
	return (filelist[ locs[0] ].filedata);
      }
    else
      {
	fprintf(stderr, "WARNING: loading memfile from memfilesystem [%s], multiple copies found (%ld). Will return first instance.\n", fname.c_str(), locs.size() );
	return (filelist[ locs[0] ].filedata);
      }
  }
  
  void _internal_write( const size_t& fidx, const std::vector<char>& towrite, bool append=true )
  {
    filelist[fidx].write( towrite, append );
  }
  
  void write_file( const std::string& fname, const std::vector<char>& towrite, bool append=true )
  {
    std::vector<size_t> locs = find_string_in_memfile_vect( fname );
    if( locs.size() == 0 )
      {
	//(attempt to) Open file from actual disk, and read it into std::vector<char>, and return that. In other words, user ONLY uses this
	//to access files -- nothing else.
	//We want to basically redefine ifstream, ofstream, and fstream... and handle all that shit. I.e. overload them. For now just do it my way.
	//will actually make a new mem_file with the name?
	mem_file mf( fname );
	mf.write( towrite, append );
	filelist.push_back( mf );
      }
    else if( locs.size() == 1 )
      {
	filelist[ locs[0] ].write( towrite, append );
      }
    else
      {
	fprintf(stderr, "WARNING: write append to memfilesystem [%s], multiple copies found (%ld). Will return first instance.\n", fname.c_str(), locs.size() );
	filelist[ locs[0] ].write( towrite, append );
	
      }
  }

  void add_file_from_disk( const std::string& fname )
  {
    add_file( mem_file( fname ) );
  }
  
  void add_file( const mem_file& mf )
  {
    filelist.push_back( mf );
  }

  memfile_ptr get_ptr( const std::string& fname )
  {
    std::vector<size_t> locs = find_string_in_memfile_vect( fname );
    if( locs.size() != 1 )
      {
	fprintf(stderr, "ERROR in get_ptr in memfile, more than one or zero files of target name [%s] (%ld) (files size: [%ld])\n", fname.c_str(), locs.size(), filelist.size());
	for(size_t x=0; x<filelist.size(); ++x)
	  {
	    fprintf(stdout, "[%s]\n", filelist[x].filename.c_str());
	  }
	exit(1);
      }

    size_t idx = locs[0];

    return memfile_ptr( filelist[idx] );
  } //end get_ptr
  
  
};

