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
  //fprintf(stdout, "Contains (no change?) [%s]\n", sf.getdata().c_str() );
  fprintf(stdout, "Got out: int should be 2 [%d] double (as str) should be 32.3 [%s], str should be YOLO: [%s]\n", myint, doubletmpstr.c_str(), mystr.c_str() );

  fprintf(stdout, "After extraction (should be empty?) [%s]. Now adding via printf\n", sf.getdata().c_str() );
  //WRiting more using printf.
  sf.printf("%d %f\n", 10, 15.1);

  int i2;
  float f2;
  sf >> i2 >> f2;
  fprintf(stdout, "Should have got 10 and 15.1: [%d] [%f]\n", i2, f2 );


  //I should consume?
  sf.printf("%ld %ld %s\n", 222, 233, "CHAR");
  fprintf(stdout, "Current unprocessed portion of file: [%s]\n", sf.getnextdata().c_str() );

  int i3, i4;
  std::string s3("NOTA");
  sf.scanf("%ld %ld %s\n", &i3, &i4, s3.data() );

  fprintf(stdout, "Current unprocessed portion of file: [%s]\n", sf.getnextdata().c_str() );

  fprintf(stdout, "I should have got: 11 (or 12 if include newline)\n");
  
  //No, it's  not empty. Shit.
  
  //Try to peek?
  if( sf.eof() )
    {
      fprintf(stdout, "Correctly EOF!\n");
    }
  else
    {
      fprintf(stdout, "Incorrectly NOT EOF!\n");
    }
  int a;
  
  sf >> a;
  //Should be EOF?
  if( sf.eof() )
    {
      fprintf(stdout, "Correctly EOF!\n");
    }
  else
    {
      fprintf(stdout, "Incorrectly NOT EOF!\n");
    }

  
  
}
