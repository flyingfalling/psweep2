
//#pragma once

#include <string_manip.h>

std::string CONCATENATE_STR_ARRAY(const std::vector<std::string>& arr, const std::string& sep)
{
  if(arr.size() > 0 )
    {
      std::string ret=arr[0];
      for(size_t a=1; a<arr.size(); ++a)
	{
	  ret += sep + arr[a];
	}
      return ret;
    }
  else
    {
      return "";
    }
}

std::vector<std::string> tokenize_string(const std::string& src, const std::string& delim, bool include_empty_repeats)
{
  std::vector<std::string> retval;
  boost::char_separator<char> sep( delim.c_str() );
  boost::tokenizer<boost::char_separator<char>> tokens(src, sep);
  for(const auto& t : tokens)
    {
      retval.push_back( t );
    }
  return retval;
}


//REV: comment out to prevent confusion on which version of tokenize_string is being called
/* keep empty, i.e. do I want to know when it is e.g. :: or //? */
// std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats)
// {
//   std::vector<std::string> res;
  
//   /* REV: size_t is uint? */
//   size_t prev = 0;
//   size_t next = 0;

//   /* npos is -1 */
//   while ((next = source.find_first_of(delim, prev)) != std::string::npos)
//     {
//       if (include_empty_repeats || ((next-prev) != 0) )
// 	{
// 	  res.push_back(source.substr(prev, (next-prev)) );
// 	}
//       prev = next + 1;
//     }

//   /* REV: push back remainder if there is anything left (i.e. after the last token?) */
//   if (prev < source.size())
//     {
//       res.push_back(source.substr(prev));
//     }

//   return res;
// }


std::string get_canonical_dir_of_fname( const std::string& s, std::string& fnametail )
{
  std::stack<std::string> fnstack;

  //This will go through canonicalized and push back... I.e. stack contains
  //FIRST one DEEPEST.

  //FIRST POP should have FILENAME
  
  //Only works on LINUX/UNIX? That begin with root assuming /
  //Otherwise, windows would be like C:// etc.
  bool isglobal=false;

  if( s.size() < 1 )
    {
      fprintf(stderr, "REV: error, filename of string in get_canonical_dir_of_fname is empty... [%s]\n", s.c_str() );
      exit(1);
    }
  
  //First, remove all whitespace? No.
  //REV: note this ignores all escape characters...
  if( s[0] == '/' )
    {
      isglobal = true;
    }
  
  //first, tokenize it by /  (how to handle first one?)
  std::vector<std::string> vect = tokenize_string( s, "/");
  
  //Then, iterate through from beginning, push back thing to a stack. If .., pop the stack. If stack size < 0, error.
  //REV: error...if first one is ../, what will happen? If stack size is zero, I just leave it...
  //bool leading_doubledots = true;

  //e.g. ../blah/lah
  //e.g. ./../lah
  //e.g. ./../mydir/../asdf
  //Negative stack stores number of steps "below" zero.

  //e.g., ./../blah/../blah2
  //Will go to:
  //1
  //blah
  //1 (popped blah)
  //blah2

  //e.g. ./../../blah/../../blah3
  //1
  //2
  //blah
  //2 (popped blah)
  //3
  //blah3.
  //end up as:
  //../../../blah3
  
  size_t negative_stack_size = 0;
  
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( vect[x].compare("..") == 0 )
	{
	  if( fnstack.size() == 0 )
	    {
	      ++negative_stack_size;
	    }
	  else
	    {
	      fnstack.pop();
	    }
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

  fnametail = fnstack.top();
  fnstack.pop(); //REV: 3 mar 2017: This is pop_back right?
  
  std::vector<std::string> ret( fnstack.size() );
  
  
  //REV: just reversing everything
  //REV: going backwards so size is "stuck" (not checking against new ret.size() every time)
  for(size_t x=ret.size(); x>0; --x)
    {
      ret[ x-1 ] = fnstack.top();
      fnstack.pop();
    }

  
  std::string finalstring =  CONCATENATE_STR_ARRAY( ret, "/" );
  
  
  if(isglobal)
    {
      //if global and non-empty, it's the trailing slash that will be added when user does dir+"/"+fname, not the leading one, which wasn't added by
      //the CONCAT_STR_ARRAY above. So, we add it manually
      if( finalstring != "" )
	{
	  finalstring = "/" + finalstring;
	}
    }
  else
    {
      std::string doubledotstr = "";
      for(size_t x=0; x<negative_stack_size; ++x)
	{
	  doubledotstr = doubledotstr + "/..";
	}
      //REV: just catching case where finalstring is not empty, we want to put a / between ./../.. etc. and /finalstring
      if( finalstring != "" )
	{
	  finalstring = "/" + finalstring;
	}
      finalstring = "." + doubledotstr + finalstring;
    }

  //REV: this is dir
  return finalstring;
    
  //Ending is the filename or dir name. Note remove all double or multiple slashes // etc.
    
}


std::string canonicalize_fname( const std::string& s )
{
  std::string fname;
  std::string dir = get_canonical_dir_of_fname( s, fname );

  std::string finalstring = dir + "/" + fname;
  
  return finalstring;
  
}


bool same_fnames(const std::string& fname1, const std::string& fname2)
{
  std::string s1 = canonicalize_fname( fname1 );
  std::string s2 = canonicalize_fname( fname2 );
  if( s1.compare( s2 ) == 0 )
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
      //fprintf(stdout, "Replacing OLD: [%s] with NEW [%s]\n", fvect[ replace_locs[ x ] ].c_str(), newfname.c_str());
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
      //fprintf(stderr, "Comparing files: [%s] and [%s] for equality!\n", canonical.c_str(), fname.c_str() );
      if( same_fnames( canonical, fname ) == true )
	{
	  //fprintf(stderr, "FOUND IT! [%s]\n", fname.c_str());
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
