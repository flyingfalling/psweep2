#include <searchalgos.h>
#include <variable.h>
#include <searcher.h>



int main()
{
  std::string searchalg = "grid";
  std::string minmaxstepfile = "testminmaxstep.bounds";

  varlist<std::string> vl;
  vl.addvar( variable<std::string>( "GRID_MIN_MAX_STEP_FILE", minmaxstepfile ) );

    
  //no other vars to set?
  
  std::string scriptfname = "../configs/test_parampoint.cfg";
  std::string mydir = "./testdir";
  
  run_search( searchalg, scriptfname, mydir, vl );
    
}
