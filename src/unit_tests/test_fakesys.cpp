
#include <filesender.h>

//REV: Testing fake system. Will do same sweep, but this time, with fake system.

void user_funct( const std::vector<std::string>& args, mem_filesystem& fsys );

void user_funct( const std::vector<std::string>& argv, mem_filesystem& fsys )
{
  std::string ftoparse="SUPERERROR";
  std::string outputf="ERROROUTPUT";

  if(argv.size() != 4 )
    {
      fprintf(stderr, "REV: In user funct, error, argv length is not 4\n");
      exit(1);
    }
  //REV: treat it same as argv, where 0 is called function name??? OK. Which way to do it? Meh, ignore it.
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


  //And the actual one.
  varlist<std::string> vl;
  //REV: Haha you sure dug yourself into a hole here...crap. Anything that wants to read from file has to be re-written to accept a mem_file...
  //Easiest way is to make some "global" mem_filesystem...
  vl.inputfromfile( ftoparse );
  
  //std::ofstream of;
  //open_ofstream( outputf,  of );

  
  for(size_t x=0; x<vl.vars.size(); ++x)
    {
      fprintf(stdout, "  USER PRGRM: varname [%s], val [%s] to decimal\n", vl.vars[x].name.c_str(), vl.vars[x].get_s().c_str() );
      //vl.vars[x].val = std::to_string( std::stod( vl.vars[x].get_s() ) * 5 );
    }

  vl.tofile( outputf );
  
  
}

int main()
{
  
}
