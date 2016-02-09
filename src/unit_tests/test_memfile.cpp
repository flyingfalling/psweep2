#include <iostream>
#include <cstdlib>

#include <memfile.h>
#include <utility_functs.h>

#include <fstream>

int main()
{
  mem_filesystem fs;
  //REV: Make fake filesystem

  std::ofstream bin( "memfiletest.binary", std::ios_base::binary );
  double xd=3.0, yd=2.5, zd=5.0;
  int xi=1, yi=15, zi=30000;
  long int xl=5999333, yl=1, zl=483;

  bin << xd << yd << zd << xi << yi << zi << xl << yl << zl;

  bin.close();

  std::ofstream txt( "memfiletest.txt" );

  txt << xd << " " << yd << " " << zd << " " << xi << " " << yi << " " << zi <<
    " " << xl << " " << yl << " " << zl; //no endl.

  txt.close();
  
  //make a double then int file.
  fs.add_file_from_disk( "memfiletest.binary" );
  fs.add_file_from_disk( "memfiletest.txt" );
  
  memfile_ptr binptr = fs.get_ptr( "memfiletest.binary" );
  memfile_ptr txtptr = fs.get_ptr( "memfiletest.txt" );

  fprintf( stdout, "From binary: %ld %ld %ld %d %d %d %ld %ld %ld\n", binptr.consume_from_binary<double>(), binptr.consume_from_binary<double>(), binptr.consume_from_binary<double>(), binptr.consume_from_binary<int>(), binptr.consume_from_binary<int>(), binptr.consume_from_binary<int>(), binptr.consume_from_binary<long int>(), binptr.consume_from_binary<long int>(), binptr.consume_from_binary<long int>() );

  std::istringstream iss = txtptr.get_string_stream();

  double ds[3];
  int di[3];
  long int dl[3];

  
  iss >> xd  >> yd  >> zd  >> xi  >> yi  >> zi >>
    " " >> xl  >> yl  >> zl;

  fprintf(stdout, "FROM TXT:\n");
  std::cout << xd << " " << yd << " " << zd << " " << xi << " " << yi << " " << zi << " " << xl << " " << yl << " " << zl << std::endl;

}
