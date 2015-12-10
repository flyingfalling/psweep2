#include <variable.h>


main()
{

  varlist<std::string> vl;
  vl.addTvar( "V1", "YOLOV1");
  vl.addTvar( "V2", "YOLOV2");
  std::vector<std::string> arr;
  arr.push_back( "AV1" );
  arr.push_back( "AV2" );
  vl.addArrayvar( "V2", arr );

  fprintf(stdout, "Enumerating it:\n");
  vl.enumerate();
}
