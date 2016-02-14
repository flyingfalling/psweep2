
//#include <filesender.h>
//#include <fake_system.h>
#include <searcher.h>
//REV: Testing fake system. Will do same sweep, but this time, with fake system.

//void user_funct( const std::vector<std::string>& argv, mem_filesys& fsys );

void user_funct( const std::vector<std::string>& argv, memfsys& fsys )
{
  std::string ftoparse="SUPERERROR";
  std::string outputf="ERROROUTPUT";

  if(argv.size() != 5 )
    {
      fprintf(stderr, "REV: In user funct, error, argv length is not 4\n");
      exit(1);
    }
  //REV: treat it same as argv, where 0 is called function name??? OK. Which way to do it? Meh, ignore it.
  if( argv[0].compare( "-c" ) != 0 )
    {
      fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[1].c_str());
    }
  ftoparse = std::string(argv[2]);
  fprintf(stdout, "TEST PROGRAM: USING VAR FILE [%s]\n", ftoparse.c_str());

  if( argv[2].compare( "-o" ) != 0 )
    {
      fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[3].c_str());
    }

  outputf = std::string(argv[4]);
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

  //REV: WOw this is horribly ugly...I need a better way of interacting with numbers in varlists...
  vl.setvar( "VAR1", variable<std::string>("VAR1", std::to_string(std::stod(vl.getvar( "VAR1" ).get_s())*1000.0 ))  );
  
  //Writing not to memfile? How to figure it out.
  vl.tofile( outputf, fsys, false );
}



int main(int argc, char** argv)
{
  bool runfile=false;
  if (argc > 1)
    {
      if( strcmp(argv[1], "file") == 0 )
	{
	  runfile=true;
	}
    }
  
  std::string minmaxstepfile = "testminmaxstep.bounds";
  varlist<std::string> paramsvl;
  paramsvl.addvar( variable<std::string>( "GRID_MIN_MAX_STEP_FILE", minmaxstepfile ) );
  
  std::string searchalg = "grid";
  std::string scriptfname = "../configs/test_fakesystem.cfg";
  std::string mydir = "./testdir2";
  
  
  if( !runfile )
    {
      searcher srch1;
      srch1.register_funct( "./userfunct", user_funct );
      bool writefiles=false;
      srch1.run_search( searchalg, scriptfname, mydir, paramsvl, writefiles);
  
      //It called "exit" after search, but I'm still at main for main loop. OK.
      fprintf(stdout, "After search (IN MEMORY!!!): Parampoint generator got [%ld] PP results\n", srch1.pg.parampoint_results.size());
      for(size_t x=0; x<srch1.pg.parampoint_results.size(); ++x)
	{
	  fprintf(stdout, "Results varlist for pitem 0 of last PSET of PP [%ld]:\n", x);
	  varlist<std::string> r = srch1.pg.get_result( x, srch1.pg.parampoint_results[x].pset_results.size()-1, 0 );
      
	  r.enumerate();
	}
    }
  else
    {
      fprintf(stdout, " - - - - - OMG RUNNING WITH FILES\n");
      bool writefiles = true;
      std::string mydir = "./testdir";
      std::string scriptfname = "../configs/test_parampoint.cfg";
      
      searcher srch2;
      srch2.run_search( searchalg, scriptfname, mydir, paramsvl, writefiles);
      fprintf(stdout, "After search: Parampoint generator got [%ld] PP results\n", srch2.pg.parampoint_results.size());
      for(size_t x=0; x<srch2.pg.parampoint_results.size(); ++x)
	{
	  fprintf(stdout, "Results varlist for pitem 0 of last PSET of PP [%ld]:\n", x);
	  varlist<std::string> r = srch2.pg.get_result( x, srch2.pg.parampoint_results[x].pset_results.size()-1, 0 );
      
	  r.enumerate();
	}
    }
  
  return 0;
}
