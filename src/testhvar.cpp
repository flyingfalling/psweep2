#include <variable.h>
#include <hierarchical_varlist.h>

main()
{

  varlist<std::string> vl1("ROOTVLIST");
  vl1.addTvar( "V1", "YOLOV1");
  vl1.addTvar( "V2", "YOLOV2");
  std::vector<std::string> arr;
  arr.push_back( "AV1" );
  arr.push_back( "AV2" );
  vl1.addArrayvar( "V2", arr );

  

  varlist<std::string> vl2("LEFT1LEVEL");
  
  vl2.addTvar( "V1-1", "YOLOV1");
  vl2.addTvar( "V1-2", "YOLOV2");
  std::vector<std::string> arr2;
  arr2.push_back( "AV1-1" );
  arr2.push_back( "AV1-2" );
  vl2.addArrayvar( "V21", arr2 );

  
  varlist<std::string> vl3("LEFT2LEVEL");
  
  vl3.addTvar( "V3-1", "YOLOV1");
  vl3.addTvar( "V3-2", "YOLOV2");
  std::vector<std::string> arr3;
  arr3.push_back( "AV3-1" );
  arr3.push_back( "AV3-2" );
  vl3.addArrayvar( "V32", arr3 );

  varlist<std::string> vl4("RIGHT1LEVEL");
  
  vl4.addTvar( "V4-1", "YOLOV1");
  vl4.addTvar( "V4-2", "YOLOV2");
  std::vector<std::string> arr4;
  arr4.push_back( "AV4-1" );
  arr4.push_back( "AV4-2" );
  vl4.addArrayvar( "V42", arr4 );

  fprintf(stdout, "TESTING hierarchical varlist:\n");
  hierarchical_varlist<std::string> hvl (vl1 );

  hvl.add_child( 0, vl2 );
  hvl.add_child( 0, vl4 );
  hvl.add_child( hvl.get_children(0)[0], vl3 );


  hvl.enumerate();
  //hvl.add_child( 0, vl2 );
  
}
