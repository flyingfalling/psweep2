
#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <stack>


std::string CONCATENATE_STR_ARRAY(const std::vector<std::string>& arr, const std::string& sep)
{
  std::string ret="";
  for(size_t a=0; a<arr.size(); ++a)
    {
      ret+= arr[a] + sep;
    }
  return ret;
}


/* keep empty, i.e. do I want to know when it is e.g. :: or //? */
std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats=false)
{
  std::vector<std::string> res;
  
  /* REV: size_t is uint? */
  size_t prev = 0;
  size_t next = 0;

  /* npos is -1 */
  while ((next = source.find_first_of(delim, prev)) != std::string::npos)
    {
      if (include_empty_repeats || ((next-prev) != 0) )
	{
	  res.push_back(source.substr(prev, (next-prev)) );
	}
      prev = next + 1;
    }

  /* REV: push back remainder if there is anything left (i.e. after the last token?) */
  if (prev < source.size())
    {
      res.push_back(source.substr(prev));
    }

  return res;
}


std::string get_canonical_dir_of_fname( const std::string& s, std::string& fnametail )
{
  std::stack<std::string> fnstack;

  //Only works on LINUX/UNIX? That begin with root assuming /
  //Otherwise, windows would be like C:// etc.
  bool isglobal=false;
  
  //First, remove all whitespace? No, don't.
  if( s[0] == '/' )
    {
      isglobal = true;
    }
  
  //first, tokenize it by /  (how to handle first one?)
  std::vector<std::string> vect = tokenize_string( s, "/");
  
  //Then, iterate through from beginning, push back thing to a stack. If .., pop the stack. If stack size < 0, error.
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( vect[x].compare("..") == 0 )
	{
	  fnstack.pop();
	}
      else if( vect[x].compare(".") == 0)
	{
	  //do nothing (just remove it, no need for something that specifies "same" directory
	}
      else
	{
	  fnstack.push( vect[x] );
	}
      
    }
  std::vector<std::string> ret( fnstack.size() );
  if(fnstack.size() < 1)
    {
      fprintf(stderr, "REV: ERROR, trying to get dir of a file(name) that is HERE\n");
    }
  for(size_t x=0; x<ret.size()-1; ++x)
    {
      ret[ x ] = fnstack.top();
      fnstack.pop();
    }

  
  std::string finalstring =  CONCATENATE_STR_ARRAY( ret, "/" );

  if(isglobal)
    {
      finalstring = "/" + finalstring;
    }

  return finalstring;
    
  //Ending is the filename or dir name. Note remove all double or multiple slashes // etc.
    
}


std::string canonicalize_fname( const std::string& s )
{
  std::stack<std::string> fnstack;

  bool isglobal=false;
  
  //First, remove all whitespace? No, don't.
  if( s[0] == '/' )
    {
      isglobal = true;
    }
  
  //first, tokenize it by /  (how to handle first one?)
  std::vector<std::string> vect = tokenize_string( s, "/");

  
  
  //Then, iterate through from beginning, push back thing to a stack. If .., pop the stack. If stack size < 0, error.
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( vect[x].compare("..") == 0 )
	{
	  fnstack.pop();
	}
      else if( vect[x].compare(".") == 0)
	{
	  //do nothing (just remove it, no need for something that specifies "same" directory
	}
      else
	{
	  fnstack.push( vect[x] );
	}
      
    }
  std::vector<std::string> ret( fnstack.size() );
  for(size_t x=0; x<ret.size(); ++x)
    {
      ret[ x ] = fnstack.top();
      fnstack.pop();
    }

  //Will this work, cast to vector?
  std::string finalstring =  CONCATENATE_STR_ARRAY( ret, "/" );

  if(isglobal)
    {
      finalstring = "/" + finalstring;
    }

  return finalstring;
  
    
}


bool same_fnames(const std::string& fname1, const std::string& fname2)
{
  std::string s1 = canonicalize_fname( fname1 );
  std::string s2 = canonicalize_fname( fname2 );
  if( fname1.compare( fname2 ) == 0 )
    {
      return true;
    }
  else
    {
      return false;
    }
}


void replace_old_fnames_with_new( std::vector<std::string>& fvect, const std::string& newfname, const std::vector<size_t> replace_locs )
{
  for(size_t x=0; x<replace_locs.size(); ++x)
    {
      fvect[ replace_locs[ x ] ] = newfname;
    }
  
  return;
}

//This only matches EXACT filenames
std::vector<size_t> find_matching_files(const std::string& fname, const std::vector<std::string>& fnamevect, std::vector<bool>& marked )
{
  std::vector<size_t> foundvect;
  for(size_t x=0; x<fnamevect.size(); ++x)
    {
      std::string canonical = canonicalize_fname( fnamevect[x] );
      //if( strcmp( fnamevect[x], fname ) == 0 )
      if( same_fnames( canonical, fname ) == true )
	{
	  if(marked[x] == false)
	    {
	      foundvect.push_back( x );
	      marked[x] = true;
	    }
	  else
	    {
	      fprintf( stderr, "REV: WARNING in find_matching_filenames: RE-CHANGING already MARKED file (i.e. circular renaming?) File: [%s]\n",
		       fname.c_str() );
	    }
	}
      
      
    }
  return foundvect;
}
