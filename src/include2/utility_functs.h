//REV: some utility functs (wrappers) to deal with some common problems especially with lustre etc.

#pragma once

#ifndef UTILITY_FUNCTS_H
#define UTILITY_FUNCTS_H


//for errno
#include <errno.h>
#include <cstdlib>
#include <cstdio>
//for std::string
#include <string>
# include <iostream>
# include <fstream>
# include <iomanip>
#include <unistd.h> 
#include <sys/types.h>
#include <cstring>

inline void open_ifstream( std::string fname, std::ifstream& f, std::ios_base::openmode mode, size_t tries=10, long timeout_us=10000 )
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

inline void open_ofstream( std::string fname, std::ofstream& f, std::ios_base::openmode mode, size_t tries=10, long timeout_us=10000 )
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
inline void open_fstream( std::string fname, std::fstream& f, std::ios_base::openmode mode, size_t tries=10, long timeout_us=10000 )
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

inline void close_ifstream( std::ifstream& f, size_t tries=10, long timeout_us=10000 )
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

inline void close_ofstream( std::ofstream& f, size_t tries=10, long timeout_us=10000 )
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

inline void close_fstream( std::fstream& f, size_t tries=10, long timeout_us=10000 )
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

inline FILE* fopen2(const char* fn, std::string m, size_t tries=10, long timeout_us=10000 )
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

inline void fclose2( FILE* f, size_t tries=10, long timeout_us=10000 )
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


#endif //UTILITY_FUNCTS_H
