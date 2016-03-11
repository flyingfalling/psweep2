//REV: some utility functs (wrappers) to deal with some common problems especially with lustre etc.

#pragma once

#include <utility_functs.h>

void copy_file(const std::string& src, const std::string& targ)
{
  std::ifstream source(src, std::ios::binary);
  std::ofstream dest(targ, std::ios::binary);
  
  dest << source.rdbuf();
  
  source.close();
  dest.close();

  return;
}

void copy_fileBAD( const std::string& src, const std::string& targ )
{
  int source = open(src.c_str(), O_RDONLY, 0);
  int dest =   open(targ.c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);
  //int dest =   open(targ.c_str(), O_WRONLY |  O_TRUNC, 0644);

  // struct required, rationale: function stat() exists also
  struct stat stat_source;
  fstat(source, &stat_source);

  sendfile(dest, source, 0, stat_source.st_size);
  
  close(source);
  close(dest);

  return;
}


std::vector<size_t> find_string_in_vect( const std::string& targ, const std::vector<std::string>& vect )
{
  std::vector<size_t> locs;
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( targ.compare( vect[x] ) == 0 )
	{
	  locs.push_back( x );
	}
    }

  return locs;
  
}


bool check_file_existence( const std::string& fname )
{
  //if( access( fname.c_str(), F_OK ) != -1 )
  struct stat buffer;   
  if(stat (fname.c_str(), &buffer) == 0)
    {
      return true;
    }
  else
    {
      return false;
    }
}


//Gets current user's mask?
mode_t getumask()
{
    mode_t mask = umask(0);
    umask (mask);
    return mask;
}

bool make_directory( const std::string& s )
{
  if( check_file_existence( s ) == false )
    {
      
      //int res = mkdir( s.c_str(),  getumask() );
      int res = mkdir( s.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH  );
      
      if(res != 0)
	{
	  //REV: might want to try a few times or something? Or sleep or something? Ugh...
	  fprintf(stderr, "ERROR: make_directory: failed to make directory [%s]\n", s.c_str());
	  return false; //failed to make dir..?
	}
    }
  
#if DEBUGLEVEL>5
  else
    {
      fprintf(stderr, "WARNING in make_directory: trying to create already existing directory [%s]\n", s.c_str() );
    }
#endif
  return true; //no point in returning at all -_-;
}

//REV: ghetto function to read a whole file into a single std::string
std::string get_file_contents(const std::string filename)
{
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (in)
    {
      std::string contents;
      in.seekg(0, std::ios::end);
      contents.resize(in.tellg());
      in.seekg(0, std::ios::beg);
      in.read(&contents[0], contents.size());
      in.close();
      return(contents);
    }
  else
    {
      fprintf(stderr, "Could not open file stream to [%s]. Will probably throw an exception now...(REV: I will exit first)\n", filename.c_str() );
      exit(1);
    }
  throw(errno);
}

//REV: modified this to remove "mode" since it will always just be READ?
void open_ifstream( std::string fname, std::ifstream& f, std::ios_base::openmode mode, size_t tries, long timeout_us )
{
  f.open(fname);
  size_t i=0;
  if(!f.is_open())
    {
      while(!f.is_open() && i < tries)
        {
          ++i;
          f.open(fname);
	  if(!f.is_open())
	    {
              usleep(timeout_us);
            }
        }
      if(!f.is_open()) //If *STILL* fail!
	{
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR OPENING ifstream (after [%ld] tries): [%s]\n\n\n\n",
		  tries, fname.c_str());
	  fflush(stderr);
	  exit(1); //nasty way to exit. Some better way to signal other guys?
        }
    };
  
  //return f;
}

//void open_ofstream( std::ofstream& f, ios_base::openmode mode )

//inline void open_ofstream( std::string fname, std::ofstream& f, std::ios_base::openmode mode=std::ios_base::app, size_t tries=10, long timeout_us=10000 )
void open_ofstream( std::string fname, std::ofstream& f, std::ios_base::openmode mode, size_t tries, long timeout_us)
{
  //std::ofstream f;
  f.open(fname); //, mode);
  size_t i=0;
  if(!f.is_open())
    {
      while(!f.is_open() && i < tries)
        {
          ++i;
          f.open(fname);
	  if(!f.is_open())
	    {
              usleep(timeout_us);
            }
        }
      if(!f.is_open()) //If *STILL* fail!
	{
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR OPENING ofstream (after [%ld] tries): [%s]\n\n\n\n",
		  tries, fname.c_str());
	  fflush(stderr);
	  exit(1); //nasty way to exit. Some better way to signal other guys?
        }
    };
  
  //return f;
  
}

//void open_fstream( std::fstream& f, ios_base::openmode mode )
void open_fstream( std::string fname, std::fstream& f, std::ios_base::openmode mode, size_t tries, long timeout_us )
{
  //std::fstream f;
  f.open(fname, mode);
  size_t i=0;
  if(!f.is_open())
    {
      while(!f.is_open() && i < tries)
        {
          ++i;
          f.open(fname, mode);
	  if(!f.is_open())
	    {
              usleep(timeout_us);
            }
        }
      if(!f.is_open()) //If *STILL* fail!
	{
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR OPENING fstream (after [%ld] tries): [%s]\n\n\n\n",
		  tries, fname.c_str());
	  fflush(stderr);
	  exit(1); //nasty way to exit. Some better way to signal other guys?
        }
    };
  
  //return f;
  
}

void close_ifstream( std::ifstream& f, size_t tries, long timeout_us )
{
  f.close();
  size_t i=0;
  if(f.is_open())
    {
      while(f.is_open() && i < tries)
        {
          ++i;
          f.close();
	  if(f.is_open())
	    {
              usleep(timeout_us);
            }
        }
      if(f.is_open()) //If *STILL* fail!
	{
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR *CLOSING* ifstream (after [%ld] tries). Not sure what else to do, so exiting...?\n\n\n\n",
		  tries);
	  fflush(stderr);
	  exit(1); //nasty way to exit. Some better way to signal other guys?
        }
    };
  
}

void close_ofstream( std::ofstream& f, size_t tries, long timeout_us )
{
   f.close();
  size_t i=0;
  if(f.is_open())
    {
      while(f.is_open() && i < tries)
        {
          ++i;
          f.close();
	  if(f.is_open())
	    {
              usleep(timeout_us);
            }
        }
      if(f.is_open()) //If *STILL* fail!
	{
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR *CLOSING* ofstream (after [%ld] tries). Not sure what else to do, so exiting...?\n\n\n\n",
		  tries);
	  fflush(stderr);
	  exit(1); //nasty way to exit. Some better way to signal other guys?
        }
    };
}

void close_fstream( std::fstream& f, size_t tries, long timeout_us )
{
   f.close();
  size_t i=0;
  if(f.is_open())
    {
      while(f.is_open() && i < tries)
        {
          ++i;
          f.close();
	  if(f.is_open())
	    {
              usleep(timeout_us);
            }
        }
      if(f.is_open()) //If *STILL* fail!
	{
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR *CLOSING* fstream (after [%ld] tries). Not sure what else to do, so exiting...?\n\n\n\n",
		  tries);
	  fflush(stderr);
	  exit(1); //nasty way to exit. Some better way to signal other guys?
        }
    };
}

FILE* fopen2(const char* fn, std::string m, size_t tries, long timeout_us )
{
  FILE* r = fopen(fn, m.c_str());
  size_t i=0;
  if(r == NULL) //Or, !r?
    {
      while(r == NULL && i < tries)
        {
          ++i;
          r = fopen(fn, m.c_str());
          if(r == NULL)
            {
              usleep(timeout_us);
            }
        }

      if(r==NULL)
	{
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR OPENING FILE (after [%ld] tries): %s (errno=%d (%s))\n\n\n\n",
		  tries, fn, errno, strerror(errno));
	  fflush(stderr);
	  exit(1); //nasty way to exit. Some better way to signal other guys?
        }
    }
  return r;
}

void fclose2( FILE* f, size_t tries, long timeout_us )
{
  int r = fclose( f );
  size_t i=0;
  if(r == EOF)
    {
      while(r == EOF && i < tries)
        {
          ++i;
          r = fclose( f );
          if(r == EOF)
            {
              usleep(timeout_us);
            }
        }
    }
  if(r==EOF)
    {
      fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR **CLOSING** A FILE! (after [%ld] tries): (errno=%d (%s))\n\n\n\n",
	      tries, errno, strerror(errno));
      fflush(stderr);
      exit(1); //nasty way to exit. Some better way to signal other guys?
    }
  
}


