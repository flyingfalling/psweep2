//To test full paramsweep library...


#include <psweep2.h>

#include <unit_tests/cuda_prog.h>

//User has implemented their function on the GPU
//Problem is how to specify device. Best option is to do it when we farm out the workers
//Depending on which machine it is on, and how it samples, it will get GPUs differently.
//Have them (workers) communicate/broadcast which GPU to get, one at a time, until there are none left.
//Start with worker 1. How will I know the GPU is "claimed"?

//Basically, user_funct can call get_host_idx(), which will return local idx of the host they are on.
//From that, user can compute the GPU etc. to use.
//All workers have it. I expose it to the user via psweep2 global function...that's a hack.
//How do I specify a GPU device? Leave it up to user? If he e.g. uses command line. Yea.
//In that case, screw it, I will just let him know, which GPU index he should go on? So they don't overlap.

//User can access in CMD by adding the special variable __LOCALIDX

void user_funct( const std::vector<std::string>& argv, memfsys& fsys )
{
  std::string ftoparse="SUPERERROR";
  std::string outputf="ERROROUTPUT";

  if(argv.size() < 5 )
    {
      fprintf(stderr, "REV: In user funct, error, argv length is not 4 (it is [%ld])\n", argv.size());
      exit(1);
    }
  
    
  if( argv[1].compare( "-c" ) != 0 )
    {
      fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[1].c_str());
    }
  ftoparse = std::string(argv[2]);
  //fprintf(stdout, "TEST PROGRAM: USING VAR FILE [%s]\n", ftoparse.c_str());

  if( argv[3].compare( "-o" ) != 0 )
    {
      fprintf(stderr, "ERROR, unrecognized option [%s]\n", argv[3].c_str());
    }
  
  outputf = std::string(argv[4]);
  //fprintf(stdout, "TEST PROGRAM: USING OUTPUT FILE [%s]\n", outputf.c_str());

  std::string devnum="-1";
    size_t mydev=0;
  if(argv.size() == 7)
    {
      if( argv[5].compare( "-dev") != 0 )
	{
	  fprintf(stderr, "REV: Error in user program, didn't get expected [-dev] for device\n");
	}
      devnum=argv[6];
      fprintf( stdout, "User program dev number is [%s]\n", devnum.c_str() );
      mydev=std::stol(devnum);
    }

  std::vector<size_t> devs = find_legaldevs();

  
  if( mydev >= devs.size() )
    {
      fprintf(stderr, "BIG ERROR, not enough devices on the host you provided!!!!!! I should not have been farmed!!!\n");
      exit(1);
    }
  fprintf(stdout, "My rank is [%ld] so I should be using dev [%ld]\n", mydev, devs[mydev]);

  
  double peak1=  5.0;
  double peak2= -5.0;
  
  
  //Mins maxes will be 10, 10, etc.
  //User tries #
  varlist<std::string> vl;
  vl.inputfromfile( ftoparse, fsys, true );
  
  int64_t ndims = 20; //vl.get_int64( "NDIMS" );

  std::vector<float64_t> estimate( ndims );
  std::vector<float64_t> truemeans( ndims );


  //REV: Do this on the GPU
  double t1 = 0.0;
  double t2 = 0.0;
  
  for(size_t d=0; d<ndims; ++d)
    {
      std::string vname = "VAR_" + std::to_string( d );
      float64_t paramval = vl.get_float64( vname );
      t1 += pow ( paramval - peak1, 2 ); //center at -5 //distance from peak 1
      t2 += pow ( paramval - peak2, 2 ); //center at +5 //distance from peak 2
    }

  //REV: need to make both models "performant"
  //t1 = log( 1.0/3.0 * exp( -t1 ) );
  //t2 = log( 2.0/3.0 * exp( -t2 ) );
  t1 = sqrt(t1);
  t2 = sqrt(t2);

  double value=100000.0;
  
  if ( t1 < t2 )
    {
      value = t1;
    }
  else
    {
      value = t2;
    }
  
  varlist<std::string> newvl;
  newvl.add_float64("ERROR", value);

  newvl.tofile( outputf, fsys, false );
  
  return;
}



int main( int argc, char* argv[] )
{
  psweep2 p;
  p.register_funct( "./userfunct", user_funct );
  p.run_search( argc, argv );
  
  return 0;
}
