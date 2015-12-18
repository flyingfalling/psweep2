//REV: 15 dec 2015, adding parsing of literals...

#pragma once

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
  throw(errno);
}


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
    parser() : parser::base_type(parampoint)
    {
      using qi::lit;
      using qi::lexeme;
      using qi::raw;
      using ascii::char_;
      using ascii::string;
      using namespace qi::labels;

      using phoenix::at_c;
      using phoenix::push_back;
    
      //parampoint %= qi::eps >> *pset [push_back(at_c<0>(_val), _1)]; //side effect is pushing back to (returned) PARAMPOINT each PSET
      //parampoint %= *pset [push_back(at_c<0>(_val), _1)]; //side effect is pushing back to (returned) PARAMPOINT each PSET


      //REV: this needs to somehow return 2 strings? What...? Need to make it a struct? ugh.
      //startpset = string("PSET");
      
      /*startpset %= "PSET " >>
	lexeme[+(char_ - ' ')] >>
	" " >>
	lexeme[+(char_) - ';'] >>
	";";
      */

      //ghetto way of doing 
      parampoint =  +( pset )  [push_back(at_c<0>(_val), _1)];// >> qi::eps; //REV: making this %= causes it to parse it twice, what the hell.
      
      //R1 is the argument (inhereited?)
      endpset = "!PSET"  >> //*char_; //Need to make sure there is a character not used elsewhere to tel it to stop easily. Otherwise we'd read it and then
	//do stuff with it.
	//string("P1;") >>
	//qi::as_string[lexeme[+char_ - ';']]
	string(_r1) >>
	//REv: is there diff between double and single quotes?
	';'; //How does it know when to "stop reading"?
      
      //REV: OK I think I have to make STMNT recursive or something, in order to make this PSET thing actually return a PSET! Right now it's returning
      //nothing!
      //REV: this should "return" a pset...
      //pset = startpset [at_c<0>(_val) = _1, at_c<1>(_val) = _2]  >>

      //sstring %= qi::as_string [ lexeme [ +(~char_(' '))] ];
      //scstring %= qi::as_string [ lexeme [ +(~char_(';'))] ]; 
      
      //REV: this returns TWO strings! Ugh...
      //REV: Don't forget EPS if It7s a struct with just a container
      startpset %= "PSET" >>  qi::as_string[ lexeme[+(char_ - ',')] ] >> "," >> qi::as_string[ lexeme[+(char_ - ';')] ] >> ';';
      
      //string("PSET")  >> +(char_ - ';') >> ";"; // [ push_back(at_c(_val), _1), push_back(at_c(_val), _2) ]; // [ _val = std::vector<std::string>(_1, _2) ];
      //startpset %= "PSET" >>  sstring >> " " >> scstring >> ";"; //string("PSET")  >> +(char_ - ';') >> ";"; // [ push_back(at_c(_val), _1), push_back(at_c(_val), _2) ]; // [ _val = std::vector<std::string>(_1, _2) ];
      //Ugh, need to do stuff until I hit a space. How to do that...? Are all spaces already removed? No, doesn't seem that way?

      //REV: ONLY TO DO IS HERE!
      //startpset %= "PSET" >>  string() >> string() >> ";"; //string("PSET")  >> +(char_ - ';') >> ";"; // [ push_back(at_c(_val), _1), push_back(at_c(_val), _2) ]; // [ _val = std::vector<std::string>(_1, _2) ];

      
	//qi::eps >> string("PSET")  >> +(char_ - ';') >> ";"; // [ push_back(at_c(_val), _1), push_back(at_c(_val), _2) ]; // [ _val = std::vector<std::string>(_1, _2) ];
      
      //http://stackoverflow.com/questions/17717590/boost-spirit-qi-how-to-return-attributes-with-nabialek-trick
      //pset %= startpset [ _val = phoenix::construct<PSET>() ]; //at_c<0>(_val) = _1 ];
      //Ugh, some reason CONSTRUCT is getting EXACTLY the output of these guys lol...
      //pset = (string("PSET") >> string("P1") >> *char_) [ _val = phoenix::construct<PSET>(_1, _2) ]; //at_c<0>(_val) = _1 ];
      //pset = ( startpset >>
      //pset = ( (string("PSET") >> " " >>  qi::as_string[ lexeme[+(char_) - ';'] ] >> ";") [ _val = phoenix::construct<PSET>(_1, _2) ]  >> " " >> *char_ >>
      //pset =  (string("PSET") >> " " >>  qi::as_string[ lexeme[+(char_) - ';'] ] >> ";") [ _val = phoenix::construct<PSET>(_1, _2) ] >>
      //Ah, the blah - ';' means a string that does not include that. So once it hits that, it stops.
      //pset =  (string("PSET") >> qi::as_string[ lexeme[+(char_ - ';')] ] >> ";") [ _val = phoenix::construct<PSET>(_1, _2) ] >>
      //pset =  
      pset = ( startpset ) [ _val = phoenix::construct<PSET>( _1[0], _1[1] ) ] >
      //pset = ( startpset ) [ _val = phoenix::construct<PSET>( _1 ) ] >> //vector as arg haha
      //(string("PSET") >> qi::as_string[ lexeme[+(char_ - ';')] ] >> ";") [ _val = phoenix::construct<PSET>(_1, _2) ] >> 
	//"STMNT1;" >>
	//*char_ [push_back(at_c<2>(phoenix::construct<STMNT>(_val)), _1)] >>  //REV* need to construct statements here ... ugh.
	//*( ( qi::as_string[lexeme[+(char_ - ';' - '!')] ]  >>  ";")  [push_back(at_c<2>(_val), _1)]) >> //REV* need to construct statements here ... ugh.
	//* (qi::as_string[ lexeme[+(char_ - '(' - ';' - '!')] ] [push_back(at_c<2>(_val), _1)]  >> "(" >> qi::as_string[ lexeme[ +(char_ - ')') ]][push_back(at_c<2>(_val), _1)] >> ")" >> ";" ) >>
	//* (qi::as_string[ lexeme[+(char_ - '(' - ';' - '!')] ] [push_back(at_c<2>(_val), _1)]  >> "(" >>  lexeme[ +(char_ - ')') ] >> ")" >> ";" ) >>
      *( stmnt  [push_back(at_c<2>(_val), _1)] ) > //REV* need to construct statements here ... ugh.
	//REV: PROBLEM, this is eating the end guy too...maybe that's the problem.
        //*stmnt [push_back(at_c<2>(_val), _1)]  >>  //REV: so this POS is returning some bullshit that is fucking everything up.
	//*char_;
	endpset(at_c<0>(_val));//when is everything parsed?
      //       ; //at_c<0>(_val) = _1 ];

      //stmnt =   (qi::as_string[ lexeme[+(char_ - '(' - ';' - '!')] ] >> "(" >>  lexeme[ +(char_ - ')') ] >> ")" >> ";" );
      
      //REV: do double arrows always require "space" in between them/
      
      
      //REV: construct an empty one?
      //REV: is this only returning the first one?

      //REV: this won't work, need to specify one or none of the other guy, followed by any number of comma only guys?

      //Specify this does not start with a "!"
      //OK, the !lit statement handles this
      stmnt = ( !qi::lit('!') > fname [at_c<0>(_val) = _1]  > '(' >
		arglist [at_c<1>(_val) = _1] > ')' > ";" ) ; // [_val = phoenix::construct<STMNT>(_1, _2)]; //add fname and arglist //2 is the vector...
      
      //only diff is that leafstmnt has no ending semicolon...
      //REV: something tells me this is reading in something it shouldnt as a leaf statement, for example funct name is the close parens or comma
      //REV: this won't work recursively...because, it will "look" for an fname in the empty space after doing the first fname...
      //I *Can't* eat the next character because that will mess everything up (the next parsing step)...
      leafstmnt = ( fname [at_c<0>(_val) = _1] >
		    "(" > arglist [at_c<1>(_val) = _1, at_c<2>(_val)=false] > ")"  ) | literal [at_c<0>(_val) = _1, at_c<2>(_val)=true]; // | (literal[at_c<0>(_val) = _1]);

      //"Stop" when I encounter my end marker quote...lol
      //literal = qi::as_string[ qi::lit('"') >  lexeme[+(char_ - '(' - ')' - ',' - '"')]  > qi::lit('"') ];
      // literal %= qi::as_string[ qi::lit('"') >  lexeme[+(char_ - '(' - ')' - ',' - '"')]  > qi::lit('"') ];
      literal = '"' >  lexeme[+(char_ - '(' - ')' - ',' - '"')]  > '"' ;
    //stmnt = (fname >> "(" >> qi::as_string[lexeme[+(char_ - ',' - ')' )] ] >> ")" >> ";") [_val = phoenix::construct<STMNT>(_1, _2)]; //add fname and arglist //2 is the vector...
    //stmnt =  qi::as_string[ lexeme[+(char_ - '(')] ]  >> "(" >> +(char_ - '(') >>")" >> ";";
    //( qi::as_string[ lexeme[+(char_ - ',' - ')' )] ] >> ")" ) [ push_back(at_c<1>(_val), _1)]  >>
    
    // [ _val = phoenix::construct<STMNT>(_1) ] 
    
    //		;      //add fname and arglist //2 is the vector...
    
  
      //Need to do either "none followed by one or more with commas"


      //REV: THIS IS THE PROBLEM, if I want to search for literals I need a different way to stop the search.
      //I dont want all literals to have to be post-ceded by ??? Ah, how about removing commas from there? OK
      //And, that way, that will be parsed as the "fname"?
      
      //REV: adding end parens here to deal with "close paren) being an fname
      //fname %= !qi::lit('"') > qi::as_string[ lexeme[+(char_ - '"' - '(' - ';' - ')')] ] ; //will not get rid of spaces...hm.
      //Need a way to specify that the quote is optional, i.e. if it exists, yay, otherwise, no.
      //This is needed to deal with "empty" lists?
      //fname %=  lexeme[-(lit('"') | lit('(')) >> +(char_ - '(' - ';' - ')')]  ; //will not get rid of spaces...hm.
      //fname %=  !qi::lit('"') > lexeme[-(lit('"') | lit('(') | lit(')')) > +(char_ - '(' - ';' - ')')]  ; //will not get rid of spaces...hm.
      fname %=  !qi::lit('"') >> lexeme[ +(char_ - '(' - ';' - ')')]  ; //will not get rid of spaces...hm.
      
      //cfname %= "," >> qi::as_string[ lexeme[+(char_ - '(' - ')')] ] ; //will not get rid of spaces...hm.
      
      //arglist = farg;
      //need to use the "empty thing" trick so that I can just push directly to container?
      //arglist = nothing | (farg >> *("," >> farg)); //a list of strings or of other things? For now just a list of strings?
      //Need a way of having an empty vector. Something rule.
      
      //TODO: REV: this needs to return list of STATEMENTS!!!!
      //REV: need to include "end parenthesis" too after all...
      //REV: this is eating up the comma... I need to remove the comma..
      //The comma is falling under this leaf statement...
      //arglist %=  *( leafstmnt | ("," >> leafstmnt) ); // [ push_back(at_c<0>(_val), _1) ]; //a list of strings or of other things? For now just a list of strings?
      
      //Will this work? Recursive kleene stars? Ah, yes it will!]
      //REV: adding 
      arglist %=  qi::repeat(0, 1)[ ((leafstmnt) > *("," > leafstmnt) ) ];
	; //It is nothing.
      // [ push_back(at_c<0>(_val), _1) ]; //a list of strings or of other things? For now just a list of strings?
      //Ugh, need to make this either an empty vector, or what?
      
      
      //nothing = qi::empty; //REV: this is not correct
      
      //TODO: THIS NEEDS TO RETURN A STATEMENT
      //farg %= qi::as_string[lexeme[+(char_ - ',' - ')' )] ];
      //farg %= qi::as_string[lexeme[+(char_ - ',' - ')' )] ];
      
      //pset = (*char_ >> " " >> *char_ >> " " >> *char_) [ _val = phoenix::construct<PSET>(_1, _2) ]; //at_c<0>(_val) = _1 ];
      //>> //Ah, I'm doing multiple things to _val in here? Is that it? NO, it should be fine!
      //	*stmnt [push_back(at_c<2>(_val), _1)] >>
      //	endpset(at_c<0>(_val));

      //REV: case where stmnt is just a string.
      //stmnt = *( qi::as_string[ ( lexeme[+(char_ - ';' - '!')]   >>  ";")]  [push_back(at_c<2>(_val), _1)]); //REV* need to construct statements here ... ugh.

      
      //stmnt = lexeme[+(char_ - ';')] >>     //lexeme[+(char_ - ';') [_val += _1]] >> //[_val=1]
      //";" ;
      //REV: where do I set "val = 1" here? This needs to return a string...

      //REV: note this can be done with some weird boost template thing instead of one for each rule...
      qi::on_error<qi::fail>
        (
	 parampoint,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail parampoint)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: \"")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("\"")
	 << std::endl
        );
      qi::on_error<qi::fail>
        (
	 literal,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail literal)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: [[")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("]]")
	 << std::endl
        );
      qi::on_error<qi::fail>
        (
	 stmnt,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail stmnt)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: \"")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("\"")
	 << std::endl
        );
      qi::on_error<qi::fail>
        (
	 pset,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail pset)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: \"")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("\"")
	 << std::endl
        );
      qi::on_error<qi::fail>
        (
	 startpset,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail startpset)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: \"")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("\"")
	 << std::endl
        );
      qi::on_error<qi::fail>
        (
	 endpset,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail endpset)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: \"")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("\"")
	 << std::endl
        );
      qi::on_error<qi::fail>
        (
	 leafstmnt,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail leafstmnt)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: \"")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("\"")
	 << std::endl
        );
      qi::on_error<qi::fail>
        (
	 fname,
	 //pset,
	 std::cout
	 << phoenix::val("Error (fail fname)! Expecting ")
	 << _4                               // what failed?
	 << phoenix::val(" here: \"")
	 << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << phoenix::val("\"")
	 << std::endl
        );
      BOOST_SPIRIT_DEBUG_NODES((pset)(literal)(stmnt)(startpset)(fname)(arglist)(endpset)(leafstmnt))
    }
  
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


//Must be recursive for statement printing...
void enum_pset( const client::PSET& p )
{
  fprintf( stdout, "#PSET (%s) parallel: (%s)\n", p.NAME.c_str(), p.NREP.c_str() );
  for( size_t s=0; s<p.CONTENT.size(); ++s)
    {
      enum_stmnt( p.CONTENT[s] );
    }
  return;
}


void enum_stmnt( const client::STMNT& s )
{
  fprintf( stdout, " --STMNT: (%s), ARGS:\n", s.TAG.c_str());
  for( size_t a=0; a<s.ARGS.size(); ++a)
    {
      enum_leaf_stmnt( s.ARGS[a], 2 );
    }
  return;
}

//REV: should be leaf stmnt only
void enum_leaf_stmnt( const client::LEAF_STMNT& s, const size_t& depth )
{
  if( s.ARGS.size() > 0 )
    {

      for(size_t tl=0; tl<depth; ++tl)
	{
	  fprintf(stdout, " |");
	}
      
      fprintf(stdout, "LSTMNT: (%s), ARGS:\n", s.TAG.c_str());
      for( size_t c=0; c<s.ARGS.size(); ++c)
	{
	  enum_leaf_stmnt( s.ARGS[c], depth+1 );
	}
      return;
    }
  else
    {
      for(size_t tl=0; tl<depth; ++tl)
	{
	  fprintf(stdout, " |");
	}

      if( s.ISLIT )
	{
	  fprintf( stdout, "*** LITERAL LEAF: (%s)\n", s.TAG.c_str());
	}
      else
	{
	  fprintf( stdout, "NON-LITERAL LEAF: (%s)\n", s.TAG.c_str());
	}
      return;
    }
  
}


client::PARAMPOINT parse_psweep_script( std::string input )
{
  fprintf(stdout, "Parsing [%s]\n", input.c_str());
  auto f(std::begin(input)), l(std::end(input)); //declare an f and l, which are beginning and end of input.
  const static client::parser<decltype(f)> p;
  client::PARAMPOINT result;
  bool ok = client::qi::phrase_parse(f,l,p,client::qi::space,result);
  if (!ok)
    {
      std::cout << "ERROR: invalid input" << std::endl;
    }
  return result;
}


