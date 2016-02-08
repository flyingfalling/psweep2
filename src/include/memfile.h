#pragma once

//REV: memory file. Holds a file in memory for ghetto way of passing to user program via memory.

//REV: Need to make it archivable, so we can easily do the stuff there.

struct mem_file
{
  std::string filename;
  std::vector< char > filedata;

  mem_file( const std::string& memfname, const std::vector<char>& content )
  {
    filedata=content;
    ilename = memfname;
  }
  
  mem_file( const std::string& fname )
  {
    // open the file:
    std::streampos fileSize;
    std::ifstream file(fname, std::ios::binary);
    
    // get its size:
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // read the data:
    //std::vector<char> fileData(fileSize);
    filedata.resize(fileSize);
    file.read(filedata.data(), fileSize);
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

  void tofile(const std::string& dir, const std::string& myfname )
    {
      //REV: this does not use the FNAME of this mem_file...? Ugh.
      std::string fullfname = dir + "/" + myfname;

      std::ofstream ofs;

      open_ofstream( fullfname, ofs );
    
      ofs.write( contents.data(), contents.size() );

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

  //std::vector<size_t> find_string_in_vect( std::string&
  
  std::vector<char> read_file( const std::string& fname )
  {
    std::vector<size_t> locs = find_string_in_vect( fname, filelist );
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
    std::vector<size_t> locs = find_string_in_vect( fname, filelist );
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

  
  
  
};

