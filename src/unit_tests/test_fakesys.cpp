
#include <filesender.h>

#include <fake_system.h>
//REV: Testing fake system. Will do same sweep, but this time, with fake system.

//void user_funct( const std::vector<std::string>& argv, mem_filesys& fsys );

void user_funct( const std::vector<std::string>& argv, mem_filesys& fsys )
{
  std::string ftoparse="SUPERERROR";
  std::string outputf="ERROROUTPUT";

  if(argv.size() != 4 )
    {
      fprintf(stderr, "REV: In user funct, error, argv length is not 4\n");
      exit(1);
    }
  //REV: treat it same as argv, where 0 is called function name??? OK. Which way to do it? Meh, ignore it.
  if( argv[0].compare( "-c" ) != 0 )
    {
      fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[0].c_str());
    }
  ftoparse = std::string(argv[1]);
  fprintf(stdout, "TEST PROGRAM: USING VAR FILE [%s]\n", ftoparse.c_str());

  if( argv[2].compare( "-o" ) != 0 )
    {
      fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[2].c_str());
    }

  outputf = std::string(argv[3]);
  fprintf(stdout, "TEST PROGRAM: USING OUTPUT FILE [%s]\n", outputf.c_str());
  
  
  //And the actual one.
  varlist<std::string> vl;
  //REV: Haha you sure dug yourself into a hole here...crap. Anything that wants to read from file has to be re-written to accept a mem_file...
  //Easiest way is to make some "global" mem_filesystem...
  

  fprintf(stdout, "About to read from file...\n");
  vl.inputfromfile( ftoparse, fsys, true );
  fprintf(stdout, "Read from file...\n");
  
    
  for(size_t x=0; x<vl.vars.size(); ++x)
    {
      fprintf(stdout, "  USER PRGRM: varname [%s], val [%s] to decimal\n", vl.vars[x].name.c_str(), vl.vars[x].get_s().c_str() );
      //vl.vars[x].val = std::to_string( std::stod( vl.vars[x].get_s() ) * 5 );
    }

  vl.tofile( outputf );
  
  
}

int main()
{
  std::vector<std::string> arglist = {"-c", "testin.var", "-o", "testout.var" };
  
  
  
  mem_filesys mf;
  user_funct( arglist, mf );

  return 0;
}
