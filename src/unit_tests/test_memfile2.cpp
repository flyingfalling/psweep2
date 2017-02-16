#include <memfile2.h>


int main()
{
  mfile f;

  f << "YOLO";

  std::string fromf;
  //f.seekg(0, BOOST_IOS::beg);
  f >> fromf;
  fprintf(stdout, "OUTPUT: [%s]\n", fromf.c_str());
}
