//REV: 15 dec 2015, adding parsing of literals...

#pragma once

#include <utility_functs.h>

//#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include <fstream>
#include <string>
#include <cerrno>








namespace client
{
  namespace fusion = boost::fusion;
  namespace phoenix = boost::phoenix;
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  
  
  struct STMNT;
  struct LEAF_STMNT;
  
  //REV: I could get by with just LEAF_STMNT...easier to separate error messages with separated STMNT and LEAF_STMNT though.
  struct STMNT
  {
    std::string TAG;
    //std::vector< LEAF_STMNT > ARGS;
    //std::vector< std::string > ARGS;
    std::vector< LEAF_STMNT > ARGS;
    

    STMNT()
      : TAG("ERROR_NO_TAG")
    {
    }
    
    STMNT( const std::string t )
      : TAG(t)
    {
      //nothing to do.
    }

    //STMNT( std::string t, std::vector<std::string>& s )
  STMNT( std::string t, std::vector<LEAF_STMNT>& s)
  : TAG(t), ARGS(s)
    {
      //nothing to do.
      }
    
    /*STMNT( const std::string t, const std::string s)
      : TAG(t) //, ARGS(s)
    {
      ARGS.push_back(s);
      //nothing to do.
      }*/

    
  };

  struct LEAF_STMNT
  {
    std::string TAG;
    //std::vector< LEAF_STMNT > ARGS;
    //std::vector< std::string > ARGS;
    std::vector< LEAF_STMNT > ARGS;
    bool ISLIT;

    LEAF_STMNT()
      : TAG("ERROR_NO_TAG")
    {
    }
    
    LEAF_STMNT( const std::string t )
      : TAG(t)
    {
      //nothing to do.
    }

    //STMNT( std::string t, std::vector<std::string>& s )
    LEAF_STMNT( std::string t, std::vector<LEAF_STMNT>& s , bool _lit )
      : TAG(t), ARGS(s), ISLIT(_lit)
    {
      //nothing to do.
      }
    
    /*STMNT( const std::string t, const std::string s)
      : TAG(t) //, ARGS(s)
    {
      ARGS.push_back(s);
      //nothing to do.
      }*/

    
  };
  struct PSET
  {
    std::string NAME;
    std::string NREP; //local variable name is ?
    //std::vector< std::string > CONTENT;
    std::vector< client::STMNT > CONTENT;

    
    PSET(std::string n, std::string r)
      : NAME(n), NREP(r)
    {
      //nothing
    }

    PSET(std::vector<std::string> s)
    {
      if( s.size() != 2)
	{
	  fprintf(stderr, "ERROR in PSET constructor, length of argument 2-string vector is not 2\n");
	  exit(1);
	}

      NAME = s[0];
      NREP = s[1];
      
    }

  PSET(std::string n, std::string r, std::vector< client::STMNT >& s)
  : NAME(n), NREP(r), CONTENT(s)
    {
      //nothing
    }

  PSET()
      : NAME("ERROR_NO_NAME"), NREP("ERROR_NO_NREP")
    {
      //nothing
    }
  };

  struct PARAMPOINT
  {
    std::vector< PSET > psets;
  };

}



BOOST_FUSION_ADAPT_STRUCT(
			  client::PSET,
			  (std::string, NAME)
			  (std::string, NREP)
			  //(std::vector<std::string>, CONTENT)
			  (std::vector<client::STMNT>, CONTENT)
)

BOOST_FUSION_ADAPT_STRUCT(
			  client::PARAMPOINT,
			  (std::vector<client::PSET>, psets)
)

BOOST_FUSION_ADAPT_STRUCT(
			  client::STMNT,
			  (std::string, TAG)
			  (std::vector<client::LEAF_STMNT>, ARGS)
)

BOOST_FUSION_ADAPT_STRUCT(
			  client::LEAF_STMNT,
			  (std::string, TAG)
			  (std::vector<client::LEAF_STMNT>, ARGS)
			  (bool, ISLIT)
)


void enum_pset( const client::PSET& p );
void enum_stmnt( const client::STMNT& s );
void enum_leaf_stmnt( const client::LEAF_STMNT& s, const size_t& depth );



namespace client
{
  template <typename It, typename Skipper = qi::space_type>
  //struct parser : qi::grammar<It, PSET(), Skipper>
  struct parser : qi::grammar<It, PARAMPOINT(), Skipper>
  {
    //parser() : parser::base_type(pset)
  parser() : parser::base_type(parampoint);
   
    qi::rule< It, PARAMPOINT(), qi::space_type > parampoint;
    qi::rule< It, PSET(), qi::space_type > pset;
    //qi::rule< It, std::vector<std::string>(), qi::space_type> stmnt;
    qi::rule< It, STMNT(), qi::space_type> stmnt;
    qi::rule< It, LEAF_STMNT(), qi::space_type> leafstmnt;
    qi::rule< It, std::vector<std::string>(), qi::space_type> startpset; //REV: this needs to be a tuple of strings or something? Waht...?
    qi::rule< It, std::string(), qi::space_type> fname; //REV: this needs to be a tuple of strings or something? Waht...?
    qi::rule< It, std::string(), qi::space_type> literal; //REV: this needs to be a tuple of strings or something? Waht...?
    qi::rule< It, std::string(), qi::space_type> sstring; //REV: this needs to be a tuple of strings or something? Waht...?
    qi::rule< It, std::string(), qi::space_type> scstring; //REV: this needs to be a tuple of strings or something? Waht...?
    //qi::rule< It, std::string(), qi::space_type> farg; //REV: this needs to be a tuple of strings or something? Waht...?
    //qi::rule< It, STMNT(), qi::space_type> farg; //REV: this needs to be a tuple of strings or something? Waht...?
    //qi::rule< It, std::vector<std::string>, qi::space_type> arglist; //REV: this needs to be a tuple of strings or something? Waht...?
    //qi::rule< It, std::vector<std::string>(), qi::space_type> arglist; //REV: this needs to be a tuple of strings or something? Waht...?
    qi::rule< It, std::vector<LEAF_STMNT>(), qi::space_type> arglist; //REV: this needs to be a tuple of strings or something? Waht...?
    qi::rule< It, void(std::string), qi::space_type> endpset;
  
  };
}


client::PARAMPOINT parse_psweep_script( std::string input );


