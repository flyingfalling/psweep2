

//To compile, I need to make .o file for HDF5_collection (?) with HDF5 compiler (?)
//I.e. make a dummy CPP file.
//Then, use MPICC to compile the other stuff, and link it...?


#include <hdf5_collection.h>
//#include <dream_abc.h>
#include <searcher.h>

//User program...super ghetto. Just computes distance from true means...?
//Don't even use random haha.


void user_funct( const std::vector<std::string>& argv, memfsys& fsys )
{
  std::string ftoparse="SUPERERROR";
  std::string outputf="ERROROUTPUT";

  if(argv.size() != 5 )
    {
      fprintf(stderr, "REV: In user funct, error, argv length is not 4\n");
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
  

  double peak1=  5.0;
  double peak2= -5.0;
  
  
  //Mins maxes will be 10, 10, etc.
  //User tries #
  varlist<std::string> vl;
  vl.inputfromfile( ftoparse, fsys, true );
  
  int64_t ndims = 20; //vl.get_int64( "NDIMS" );

  std::vector<float64_t> estimate( ndims );
  std::vector<float64_t> truemeans( ndims );

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

  std::string minmaxfile = "testabc_minmax.bounds";
  std::string obsfile = "testabc_observ.data";
  varlist<std::string> paramsvl;
  paramsvl.addvar( variable<std::string>( "ABC_TEST_MIN_MAX_FILE", minmaxfile ) );
  paramsvl.addvar( variable<std::string>( "ABC_TEST_OBSERV_DATA_FILE", obsfile ) );
  
  std::string searchalg = "DREAM-ABCz";
  std::string scriptfname = "../configs/test_abc_twopeak.cfg";
  std::string mydir = "./testabctwopeakdir";
  std::string statefile = "./testabc_twopeak.state";


  if( !runfile )
    {
      searcher srch1;
      srch1.register_funct( "./userfunct", user_funct );
      bool writefiles=false;
      srch1.run_search( searchalg, scriptfname, mydir, paramsvl, writefiles);
    }
  
  //Plot the outputs ;)
}
