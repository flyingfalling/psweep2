#include <cassert>
#include <ios> // ios_base::beg
#include <string>
#include <boost/iostreams/stream.hpp>
//#include <libs/iostreams/example/container_device.hpp>
#include <test_boostio.h>

namespace io = boost::iostreams;
namespace ex = boost::iostreams::example;

int main()
{
  using namespace std;
  typedef ex::container_device<string> string_device;
  
  /*string                     one, two;
  io::stream<string_device>  io(one);
  io << "Hello World!";
  io.flush();
  io.seekg(0, BOOST_IOS::beg); // seek to the beginning
  getline(io, two);
  assert(one == "Hello World!");
  assert(two == "Hello World!");*/
  std::string emptystring;
  io::stream<string_device>  io(emptystring); //should start empty...? I don't get it.
  int i = 32;
  double d= 385.0;
  std::string s="YOLO";
  io << "Hello" << " " << i << " " << d << " " << s;
  
  //Am I at beginning of it now? I'm so confused...user might want to seekg etc.
  //I might want to get separate pointer to underlying file/string container thing...

  //print the container?
  //fprintf(stdout, "This should print the whole container? [%s]\n", io.container().c_str() );
  
  std::string hellos;
  int i2;
  double d2;
  std::string s2;
  io.seekg(0, BOOST_IOS::beg);
  io >> hellos >> i2 >> d2 >> s2;
  fprintf(stdout, "Wow got [%s] [%d] [%lf] [%s]\n", hellos.c_str(), i2, d2, s2.c_str());
  
  
  
}
