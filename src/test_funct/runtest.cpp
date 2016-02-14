
#include <cstdio>
#include <cstdlib>

#include <variable.h>

//Get input, make output, etc.

int main(int argc, char **argv)
{
  std::string ftoparse="SUPERERROR";
  std::string outputf="ERROROUTPUT";

  if(argc > 4)
    {
      if( strcmp( argv[1], "-c" ) != 0 )
	{
	  fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[1]);
	}

      ftoparse = std::string(argv[2]);
      fprintf(stdout, "TEST PROGRAM: USING VAR FILE [%s]\n", ftoparse.c_str());

      if( strcmp( argv[3], "-o" ) != 0 )
	{
	  fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[3]);
	}

      outputf = std::string(argv[4]);
      fprintf(stdout, "TEST PROGRAM: USING OUTPUT FILE [%s]\n", outputf.c_str());
    }

  varlist<std::string> vl;
  vl.inputfromfile( ftoparse );

  //std::ofstream of;
  //open_ofstream( outputf,  of );

  
  for(size_t x=0; x<vl.vars.size(); ++x)
    {
      fprintf(stdout, "  USER PRGRM: varname [%s], val [%s] to decimal\n", vl.vars[x].name.c_str(), vl.vars[x].get_s().c_str() );
      //vl.vars[x].val = std::to_string( std::stod( vl.vars[x].get_s() ) * 5 );
    }
  vl.setvar( "VAR1", variable<std::string>("VAR1", std::to_string(std::stod(vl.getvar( "VAR1" ).get_s())*1000.0 ))  );
  
  vl.tofile( outputf );
  
  
  return 0;
}
