#include <memfile3.h>

int main()
{
  ssfile sf;

  int myint=2;
  double mydouble=32.3;
  std::string mystr="YOLO";

  sf << myint << " " << mydouble << " " << mystr << std::endl;

  fprintf(stdout, "Should contain [%s]\n", sf.getdata().c_str() );

  //Get data from it? Do I need to seek from beginning?

  myint=0;
  mydouble=0;
  std::string doubletmpstr;
  mystr="ASDF";
  sf >> myint >> doubletmpstr >> mystr;
  fprintf(stdout, "Contains (no change?) [%s]\n", sf.getdata().c_str() );
  fprintf(stdout, "Got out: int should be 2 [%d] double (as str) should be 32.3 [%s], str should be YOLO: [%s]\n", myint, doubletmpstr.c_str(), mystr.c_str() );

  
}
