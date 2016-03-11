


#pragma once

#include <memfile.h>

//Problem is that I want all the user accesses on ssfile to modify ME, not it??! Haha yea, whatever though. Works.
//REV: WHat if I want to append at the end? Do I want to overwrite it (I assume so).

void memfile::waccess()
  {
    if(waccesses > 0)
      {
	fprintf(stderr, "Opening for simultaneous writes, big error!\n");
	exit(1);
      }
    ++waccesses;
  }

  void memfile::wclosed()
  {
    if(waccesses==0)
      {
	fprintf(stderr, "ERROR in closed for memfile, already 0 accesses references...\n");
	exit(1);
      }
    --waccesses;
  }

  void memfile::raccess()
  {
    ++raccesses;
  }

  void memfile::rclosed()
  {
    if(raccesses==0)
      {
	fprintf(stderr, "ERROR in closed for memfile, already 0 accesses references...\n");
	exit(1);
      }
    --raccesses;
  }

  ~memfile::memfile()
  {

    return;
    
    //Do all the natural stuff, delete the vector etc.? Need to do otherwise?
    if( raccesses != 0 )
      {
	fprintf(stderr, "REV: Massive error, I'm in desctructor of memfile, but there is still a readaccess pointer to me fname: [%s], [%ld] accesses\n", filename.c_str(), raccesses);
	exit(1);
      }
    //Do all the natural stuff, delete the vector etc.? Need to do otherwise?
    if( waccesses != 0 )
      {
	fprintf(stderr, "REV: Massive error, I'm in desctructor of memfile, but there is still a writeaccess pointer to me...\n");
	exit(1);
      }

    //~filedata();
    //~filename();
    
  }
  void memfile::tofile( const std::string& fname )
    {
      std::ofstream ofs;
      
      //Is default to append, or to overwrite?
      //REV: CHANGE TO OVERWRITE JUST IN CASE?!?!
      open_ofstream( fname, ofs, std::ios::binary | std::ios::trunc ); //Will this write binary properly?
      
      ofs.write( filedata.data(), filedata.size() );
      
      close_ofstream( ofs );
      
      return;
    }

memfile::memfile()
: filename("ERRORFNAME")
  {
  }



  //REV: This can't be right...? I want this on the PTR side I assume? Oh well.
  //If I'm writing out, I can specify to overwrite or not?
  memfile::memfile( const std::string& fname, const bool& fromfile=false )
  {
    filename = fname;
    
    //Read data from file if I specify to do so.
    if( fromfile )
      {
	std::streampos fileSize;
	std::fstream file;
	
	file = std::fstream(fname, std::fstream::in | std::ios::binary );
	
	
	// get its size:
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	
	// read the data:
	filedata.resize(fileSize);
	file.read(filedata.data(), fileSize);

	//REV: what the heck, close it since I shouldn't need it anymore. I will synch/overwrite it later if I want.
	file.close();
      }
  }
  
  
  template<class Archive>
  void memfile::serialize(Archive & ar, const unsigned int version)
  {
    ar & filename;
    ar & filedata;
  }


memfile memfile_ptr::get_memfile() const
  {
    return *mfile;
  }
  
  void memfile_ptr::clear()
  {
    failstate=false;
    badstate=false;
    eofstate=false;
    goodstate=true;
  }

  void memfile_ptr::reset()
  {
    clear();
    readpos=0;
    writepos=0;
  }
  
  memfile_ptr::memfile_ptr()
  {
    reset();
  }

  memfile_ptr::memfile_ptr( memfile& mf )
  {
    //reset();
    open( mf );
  }

  void memfile_ptr::open( memfile& mf )
  {
    mf.waccess();
    mf.raccess();
    mfile = &mf;
    reset();
    //Sets all flags
  }

  ~memfile_ptr::memfile_ptr()
  {
    if( mfile != NULL )
      {
	close();
      }
  }
  
  void memfile_ptr::close()
  {
    mfile->wclosed();
    mfile->rclosed();
    //Close doesn't really do anything, just reset pointer to NULL.
    //There should be no buffered changes.
    mfile = NULL;
  }
  
  
  bool memfile_ptr::fail()
  {
    if( failstate )
      {
	return true;
      }
    return false;
  }

  bool memfile_ptr::bad()
  {
    if(badstate)
      {
	return true;
      }
    return false;
  }

  bool memfile_ptr::eof()
  {
    if( readpos == mfile->filedata.size() )
      {
	return true;
      }
    else
      {
	return false;
      }
    
    if(eofstate)
      {
	return true;
      }
    return false;
  }


  bool memfile_ptr::good()
  {
    if( failstate || eofstate || badstate )
      {
	return false;
      }
    return true;
	
    //REV: No, check if some state is fine, based on if user has tried to use one of the stream guys but it failed heh.
    //user hasn't gotten EOF yet maybe? They need to "check" it?
    /*if( readptr < filedata.size() )
      {
	return true;
      }
    else
    {
    return false;
    }*/
  }

  
  void memfile_ptr::tofile( const std::string& fname )
  {
    mfile->tofile( fname );
  }
  
  //REV: This returns everything. I want to get "from now" type thing?
  //REV: This returns "from readpos"
  std::string memfile_ptr::getdata()
  {
    //return std::string(mfile->filedata.begin()+readpos, mfile->filedata.end() );
    return std::string(mfile->filedata.begin(), mfile->filedata.end() );
    //return _ss.str();
  }

  std::string memfile_ptr::getnextdata()
  {
    //return std::string(mfile->filedata.begin()+readpos, mfile->filedata.end() );
    return std::string(mfile->filedata.begin()+readpos, mfile->filedata.end() );
    //return _ss.str();
  }
  
  template<typename T>
  memfile_ptr& memfile_ptr::operator<<(const T& t)
  {
    std::stringstream _ss;
    _ss << t;
    std::string tmpstr = _ss.str();
    size_t offset = mfile->filedata.size();
    size_t writesize = tmpstr.size();
    //<= because e.g. if there is already [C], and writepos == 0, and writesize == 1, offset == 1,
    //I will just overwrite it.
    if( writepos + writesize <= offset )
      {
	//No need to resize
      }
    else
      {
	size_t newend = writepos + writesize;
	mfile->filedata.resize( newend );
      }
    std::copy( tmpstr.data(), tmpstr.data()+tmpstr.size(), mfile->filedata.begin()+writepos );
    writepos += writesize;
    
    return *this;

    /*
    std::stringstream _ss;
    _ss << t;
    std::string tmpstr = _ss.str();
    size_t offset = mfile->filedata.size();
    mfile->filedata.resize( mfile->filedata.size() + tmpstr.size() );
    std::copy( tmpstr.data(), tmpstr.data()+tmpstr.size(), mfile->filedata.begin()+offset );
    //mfile->filedata.push_back( tmpstr.begin(), tmpstr.end() );
    return *this;*/
  }
  
  memfile_ptr& memfile_ptr::operator<<(std::ostream& (*t)(std::ostream&))
  {
    std::stringstream _ss;
    _ss << t;
    std::string tmpstr = _ss.str();
    size_t offset = mfile->filedata.size();
    size_t writesize = tmpstr.size();
    if( writepos + writesize <= offset )
      {
	//No need to resize
      }
    else
      {
	size_t newend = writepos + writesize;
	mfile->filedata.resize( newend );
      }
    std::copy( tmpstr.data(), tmpstr.data()+tmpstr.size(), mfile->filedata.begin()+writepos );
    writepos += writesize;
    
    return *this;
    /*
    std::stringstream _ss;
    _ss << t;
    std::string tmpstr = _ss.str();
    size_t offset = mfile->filedata.size();
    mfile->filedata.resize( mfile->filedata.size() + tmpstr.size() );
    std::copy( tmpstr.data(), tmpstr.data()+tmpstr.size(), mfile->filedata.begin()+offset );
    //mfile->filedata.push_back( tmpstr.begin(), tmpstr.end() );

    return *this;
    //_ss << t;
    //return *this;
    */
  }

  memfile_ptr& memfile_ptr::operator<<(std::ios& (*t)(std::ios&))
  {
    std::stringstream _ss;
    _ss << t;
    std::string tmpstr = _ss.str();
    size_t offset = mfile->filedata.size();
    size_t writesize = tmpstr.size();
    if( writepos + writesize <= offset )
      {
	//No need to resize
      }
    else
      {
	size_t newend = writepos + writesize;
	mfile->filedata.resize( newend );
      }
    std::copy( tmpstr.data(), tmpstr.data()+tmpstr.size(), mfile->filedata.begin()+writepos );
    writepos += writesize;
    
    return *this;
  }

  memfile_ptr& memfile_ptr::operator<<(std::ios_base& (*t)(std::ios_base&))
  {
    std::stringstream _ss;
    _ss << t;
    std::string tmpstr = _ss.str();
    size_t offset = mfile->filedata.size();
    size_t writesize = tmpstr.size();
    if( writepos + writesize <= offset )
      {
	//No need to resize
      }
    else
      {
	size_t newend = writepos + writesize;
	mfile->filedata.resize( newend );
      }
    std::copy( tmpstr.data(), tmpstr.data()+tmpstr.size(), mfile->filedata.begin()+writepos );
    writepos += writesize;
    
    return *this;
  }



  //None of the other states are set?
 
  
  template<typename T>
  memfile_ptr& memfile_ptr::operator>>(T& t)
  {
    
    std::stringstream _ss( std::string( mfile->filedata.begin()+readpos, mfile->filedata.end() ) );
    
    size_t p1 = _ss.tellg();
    
    _ss >> t;

    //Wrap .good .bad .eof .fail etc. so user can use them equivalently.
    //Write READLINE etc. functions so user can use them appropriately.
    //_ss.clear() should reset all bits?? So I know for each thing if it failed, e.g. if I preiously failed to get INT bc i read in DOUBLE,
    //but now I want to try again.
    if( _ss.fail() )
      {
	failstate=true;
	badstate=true;

      }
    
    
    if( _ss.eof() ) //check if it is true
      {
	eofstate=true;
	badstate=true;
	readpos = mfile->filedata.size();
	
      }

    if( bad() )
      {
	return *this;
      }
        
    size_t p2 = _ss.tellg();
    size_t mv = p2-p1;
    readpos += mv;
    
    return *this;
  }
  
  //REV: I want to also be able to use FPRINTF, etc. with it. To do this, derive the string, and then reset it. It is copies, so very slow heh.
  //Also, open in binary mode will make it totally different. In binary mode, I will only use my personal write/read stuff.
  //What about if user wants to do write/read stuff? sscanf etc. How do we know "how far" user has gone extracting stuff? Can user "restart? Things?

  //Overload:

  //WRITE/READ/GOOD/GET/TELL/etc....man that is nasty. User might want to freely use seek type commands on the file, in which case...?
  //I should just use a local buffer? Overload streambuf? Nah just do it my way, easiest haha.

  size_t memfile_ptr::compute_new_size( const size_t& writesize )
  {
    size_t offset = mfile->filedata.size();
    if( writepos + writesize <= offset )
      {
	return offset;
	//No need to resize
      }
    else
      {
	size_t newend = writepos + writesize;
	return newend;
      }
  }
  
  std::vector<char> memfile_ptr::read( const size_t& numbytes )
  {
    size_t endpt = readpos + numbytes;
    if( endpt >= mfile->filedata.size() )
      {
	failstate=true;
	badstate=true;
	eofstate=true;
	endpt = mfile->filedata.size();
      }
    std::vector<char> ret( mfile->filedata.begin()+readpos, mfile->filedata.begin()+endpt );
    readpos += numbytes;

    return ret;
  }
  
  void memfile_ptr::write( const std::vector<char>& towrite )
  {
    //Write string data? Or we don't care what type it is haha. It will always write to WRITEPOS...

    size_t ws= compute_new_size( towrite.size() );
    mfile->filedata.resize( ws );
    std::copy( towrite.data(), towrite.data()+towrite.size(), mfile->filedata.begin()+writepos );
    writepos += towrite.size(); //Actually written heh.
    return;
  }
  
  //Where is the "readpos" and "writepos"
  template<typename... Args>
  void memfile_ptr::printf(const char* fmt, Args... args )
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

  //REV: Do readline functs etc.?
  
  //REV: Would like to write something to make sure I got an int, when I got an int, etc.
  //In my stream case, I don't return the actual stream...so It's all messed up.
  //E.g. if user tries to get it, but it doesn't get anything, set something to FALSE for them!! Yea I need to do that.
  //E.g. if they try to do s >> myint1 >> myint2, but there is only 1 int, there should only be a true thing partway through?
  //And they should be able to check state to see that it failed to get second int or something. OK.


  //REV: Better way??
  
  template <typename...Ts>
  int memfile_ptr::scanf( const char* fmt, Ts&&...ts )
  {
    std::string fmtstr = std::string(fmt);
    fmtstr+="%n";
    int ncharswritten = -1;
    std::string buffer = getnextdata();
    
    int nargswritten = std::sscanf(buffer.c_str() ,
				   fmtstr.c_str() ,
				   std::forward<Ts>(ts)... ,
				   &ncharswritten );

    if( ncharswritten < 0 )
      {
	//Huh, something is wrong. User should check how many he "should" have written heh. Oh well.
	badstate = true;
	failstate = true;
      }
    else
      {
	readpos += ncharswritten;
      }

    
    
    //REV: This won't work if it goes past EOF, so  need to handle how many were written if conspos wasn't filled becuase
    //it hit EOF partway through...
    return (nargswritten-1);
    
  }
  

  //REV: What should happen if it fails to fill one of the guys, e.g. it tries to get a DOUBLE from a STR or something?
  //REV: Scan from start, if it tries to go past, it returns number anyway?
  template<typename... Args>
  int memfile_ptr::scanf2_REV(const char* fmt, Args... args )
  {

    std::string fmtstr = std::string(fmt);
    fmtstr+="%n";
    int conspos=-1;
    std::string buffer = getnextdata();

    //args.push_back( & conspos );
    int numargswritten = std::sscanf(buffer.c_str(), fmtstr.c_str(), args...,  &conspos);

    if( conspos < 0 )
      {
	//Huh, something is wrong. User should check how many he "should" have written heh. Oh well.
	badstate = true;
	failstate = true;
      }
    //fprintf( stdout, "I consumed [%d] chars\n", conspos );

    readpos += conspos;
    
    //REV: This won't work if it goes past EOF, so  need to handle how many were written if conspos wasn't filled becuase
    //it hit EOF partway through...
    return (numargswritten-1);
    
  }
  
