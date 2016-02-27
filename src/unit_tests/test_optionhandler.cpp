#include <optionhandler.h>

void printstrvect( const std::vector<std::string>& v )
{
  for(size_t x=0; x<v.size(); ++x)
    {
      fprintf(stdout, "[%s]\n", v[x].c_str() );
    }
}



int main( int argc, char* argv[] )
{
  //convert argv to correct order
  optlist myopts( argc, argv );

  //myopts.enumerateparsed();
  
  fprintf(stdout, "HINT: CALL WITH: -m, and/or --config, and/or -nn\n\n");
  
  std::string modelopt = "m";
  std::string confopt = "config";
  std::string nneuropt = "nn";
  std::vector<std::vector<std::string> > found;

  
  found = myopts.get_opt_args( modelopt );
  fprintf(stdout, "Testing to find option [%s]. Found [%ld] of them\n", modelopt.c_str(), found.size());
  for(size_t x=0; x<found.size(); ++x)
    {
      printstrvect( found[x] );
    }

  found = myopts.get_opt_args( confopt );
  fprintf(stdout, "Testing to find option [%s]. Found [%ld] of them\n", confopt.c_str(), found.size());
  for(size_t x=0; x<found.size(); ++x)
    {
      printstrvect( found[x] );
    }

  found = myopts.get_opt_args( nneuropt );
  fprintf(stdout, "Testing to find option [%s]. Found [%ld] of them\n", nneuropt.c_str(), found.size());
  for(size_t x=0; x<found.size(); ++x)
    {
      printstrvect( found[x] );
    }
  
  fprintf(stdout, "All tests completed\n");
  return 0;
}
