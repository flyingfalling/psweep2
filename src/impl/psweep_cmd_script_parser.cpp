//REV: 15 dec 2015, adding parsing of literals...

//#pragma once

#include <psweep_cmd_script_parser.h>




namespace client
{
   template <typename It, typename Skipper>
    //parser() : parser::base_type(pset)
   parser<It,Skipper>::parser()
     : parser::base_type(parampoint)
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

      parampoint =  +( pset )  [push_back(at_c<0>(_val), _1)];

      endpset = "!PSET" >> string(_r1) >>  ';';
      
      startpset %= "PSET" >>  qi::as_string[ lexeme[+(char_ - ',')] ] >> "," >> qi::as_string[ lexeme[+(char_ - ';')] ] >> ';';
      
      pset = ( startpset ) [ _val = phoenix::construct<PSET>( _1[0], _1[1] ) ] >
	*( stmnt  [push_back(at_c<2>(_val), _1)] ) >
	endpset(at_c<0>(_val));
      
      stmnt = ( !qi::lit('!') > fname [at_c<0>(_val) = _1]  > '(' >
		arglist [at_c<1>(_val) = _1] > ')' > ";" );
      
      leafstmnt = ( fname [at_c<0>(_val) = _1] >
		    "(" > arglist [at_c<1>(_val) = _1, at_c<2>(_val)=false] > ")"  ) | literal [at_c<0>(_val) = _1, at_c<2>(_val)=true];

      literal = '"' >  lexeme[+(char_ - '(' - ')' - ',' - '"')]  > '"' ;
    
      fname %=  !qi::lit('"') >> lexeme[ +(char_ - '(' - ';' - ')')] ; 
      
      arglist %=  qi::repeat(0, 1)[ ((leafstmnt) > *("," > leafstmnt) ) ];

      
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
  
    
} //end namespace client


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


