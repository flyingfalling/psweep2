


#pragma once

#include <algorithm>
//#include <stringstream>
#include <cstdlib>
#include <cstdio>
#include <sstream>

#include <utility_functs.h>

#include <boost/mpi.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/filesystem.hpp>
#include <fstream>

#include <vector>

#include <memory>

//REV: this is same as std::stringstream, common impl.
//I want to ignore cases where it might have \0 in it if it is e.g. binary right? It won't break stringstream...?
//In case where I am accessing it with binary, I want access to the original stream...super wasteful copying.

struct memfile_ptr;

//Problem is that I want all the user accesses on ssfile to modify ME, not it??! Haha yea, whatever though. Works.
//REV: WHat if I want to append at the end? Do I want to overwrite it (I assume so).
struct memfile;

struct memfile
{
  std::vector<char> filedata;
  std::string filename;
  size_t raccesses=0;
  size_t waccesses=0;
  //REV: Keep track if I'm being accessed? Make sure not more than one at a time for writing?

  void waccess();
 

  void wclosed();
 

  void raccess();

  void rclosed();
 

  ~memfile();
 
  void tofile( const std::string& fname );
   

  memfile();



  //REV: This can't be right...? I want this on the PTR side I assume? Oh well.
  //If I'm writing out, I can specify to overwrite or not?
  memfile( const std::string& fname, const bool& fromfile=false );
  
  
  //For serialization, required to send across boost.
  friend class boost::serialization::access;
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version);
  
};








struct memfile_ptr
{
  //Could use shared but not guaranteed that other side will also allocate with shared_ptr so no-go heh;
  //Hope like hell other side will keep it open
  memfile* mfile = NULL;
  
  size_t readpos=0;
  size_t writepos=0;

  bool failstate=false;   //stream had a problem due to user function, e.g. trying to read incorrect type or something? Or going past end of file.
  bool badstate=false;  //stream has a problem, memory deallocated or something?
  bool eofstate=false;  //EOF
  bool goodstate=true;

  bool fromfile=false; //was it originally read from a file (locally?). We might want to write it back to close it...?

  memfile get_memfile() const;
 
  
  void clear();
 

  void reset();
  
  
  memfile_ptr();
  

  memfile_ptr( memfile& mf );


  void open( memfile& mf );
 

  ~memfile_ptr();
 
  
  void close();
 
  
  bool fail();
  
  bool bad();
 
  bool eof();
  


  bool good();
  

  
  void tofile( const std::string& fname );
  
  //REV: This returns everything. I want to get "from now" type thing?
  //REV: This returns "from readpos"
  std::string getdata();
  
  std::string getnextdata();
 
  template<typename T>
  memfile_ptr& operator<<(const T& t);
  
  
  memfile_ptr& operator<<(std::ostream& (*t)(std::ostream&));
 

  memfile_ptr& operator<<(std::ios& (*t)(std::ios&));
  

  memfile_ptr& operator<<(std::ios_base& (*t)(std::ios_base&));
 



  //None of the other states are set?
 
  
  template<typename T>
  memfile_ptr& operator>>(T& t);
    
  //REV: I want to also be able to use FPRINTF, etc. with it. To do this, derive the string, and then reset it. It is copies, so very slow heh.
  //Also, open in binary mode will make it totally different. In binary mode, I will only use my personal write/read stuff.
  //What about if user wants to do write/read stuff? sscanf etc. How do we know "how far" user has gone extracting stuff? Can user "restart? Things?

  //Overload:

  //WRITE/READ/GOOD/GET/TELL/etc....man that is nasty. User might want to freely use seek type commands on the file, in which case...?
  //I should just use a local buffer? Overload streambuf? Nah just do it my way, easiest haha.

  size_t compute_new_size( const size_t& writesize );
  
  std::vector<char> read( const size_t& numbytes );
 
  
  void write( const std::vector<char>& towrite );
  
  
  //Where is the "readpos" and "writepos"
  template<typename... Args>
  void printf(const char* fmt, Args... args );
  

  //REV: Do readline functs etc.?
  
  //REV: Would like to write something to make sure I got an int, when I got an int, etc.
  //In my stream case, I don't return the actual stream...so It's all messed up.
  //E.g. if user tries to get it, but it doesn't get anything, set something to FALSE for them!! Yea I need to do that.
  //E.g. if they try to do s >> myint1 >> myint2, but there is only 1 int, there should only be a true thing partway through?
  //And they should be able to check state to see that it failed to get second int or something. OK.


  //REV: Better way??
  
  template <typename...Ts>
  int scanf( const char* fmt, Ts&&...ts );
 
  

  //REV: What should happen if it fails to fill one of the guys, e.g. it tries to get a DOUBLE from a STR or something?
  //REV: Scan from start, if it tries to go past, it returns number anyway?
  template<typename... Args>
  int scanf2_REV(const char* fmt, Args... args );
  
  
};


#include <memfile.cpp>
