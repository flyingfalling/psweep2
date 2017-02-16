


#include <hdf5_collection.h>
#include <searcher.h>


int main( int argc, char** argv )
{

  std::string fname="";
  std::string outdir="";
  if (argc > 1)
    {
      fname = std::string( argv[1] );
    }
  
  if( argc > 2)
    {
      outdir = std::string( argv[2] );
    }
  else
    {
      fprintf(stderr, "Not enough args to program\n"); exit(1);
    }
  
  dream_abc_state state;

  fprintf(stdout, "About to load state of fname [%s]\n", fname.c_str() );
  //Should I load or not?
  state.load_state( fname );

  fprintf(stdout, "Loaded state, will now print to dir [%s]\n", outdir.c_str());

  size_t genskiprate = 10;
  size_t startgen=0;
  state.enumerate_X_GR_fitness( outdir, genskiprate, startgen);

  return 0;
  
}
