#include <iostream>
#include <cstdlib>

#include <memfile.h>
#include <utility_functs.h>

#include <fstream>

int main()
{
  mem_filesystem fs;
  //REV: Make fake filesystem

  std::string bname="memfiletest.binary";
  std::string tname="memfiletest.txt";
  
  std::ofstream bin( bname, std::ios::binary | std::ios::out );
  double xd=3.0, yd=2.5, zd=5.0;
  int xi=1, yi=15, zi=30000;
  long int xl=5999333, yl=1, zl=483;

  //bin << xd << yd << zd << xi << yi << zi << xl << yl << zl;
  bin.write( (char*)&xd, sizeof(xd) );
  bin.write( (char*)&yd, sizeof(yd) );
  bin.write( (char*)&zd, sizeof(zd) );
  bin.write( (char*)&xi, sizeof(xi) );
  bin.write( (char*)&yi, sizeof(yi) );
  bin.write( (char*)&zi, sizeof(zi) );
  bin.write( (char*)&xl, sizeof(xl) );
  bin.write( (char*)&yl, sizeof(yl) );
  bin.write( (char*)&zl, sizeof(zl) );

  bin.close();

  std::ofstream txt( tname );

  txt << xd << " " << yd << " " << zd << " " << xi << " " << yi << " " << zi <<
    " " << xl << " " << yl << " " << zl; //no endl.

  txt.close();
  
  //make a double then int file.
  fs.add_file_from_disk( bname );
  fs.add_file_from_disk( tname );
  
  memfile_ptr binptr = fs.get_ptr( bname );
  memfile_ptr txtptr = fs.get_ptr( tname );

  
  
  xd=binptr.consume_from_binary<double>(), yd=binptr.consume_from_binary<double>(), zd=binptr.consume_from_binary<double>(), xi=binptr.consume_from_binary<int>(), yi=binptr.consume_from_binary<int>(), zi=binptr.consume_from_binary<int>(), xl=binptr.consume_from_binary<long int>(), yl=binptr.consume_from_binary<long int>(), zl=binptr.consume_from_binary<long int>();
  //REV: HOLY SHIT, fprintf optional args are read in opposite order (maybe)!!!!!!!!!!
  
  //fprintf( stdout, "From binary: %lf %lf %lf %d %d %d %ld %ld %ld", binptr.consume_from_binary<double>(), binptr.consume_from_binary<double>(), binptr.consume_from_binary<double>(), binptr.consume_from_binary<int>(), binptr.consume_from_binary<int>(), binptr.consume_from_binary<int>(), binptr.consume_from_binary<long int>(), binptr.consume_from_binary<long int>(), binptr.consume_from_binary<long int>() );
  fprintf( stdout, "From binary: %lf %lf %lf %d %d %d %ld %ld %ld\n", xd,yd,zd,xi,yi,zi,xl,yl,zl);

  std::istringstream iss = txtptr.get_string_stream();

  double ds[3];
  int di[3];
  long int dl[3];

  
  iss >> xd  >> yd  >> zd  >> xi  >> yi  >> zi >> xl  >> yl  >> zl;

  fprintf(stdout, "FROM TXT:\n");
  std::cout << xd << " " << yd << " " << zd << " " << xi << " " << yi << " " << zi << " " << xl << " " << yl << " " << zl << std::endl;


  fs.add_file( mem_file("testout.out", false) );
  memfile_ptr outptr = fs.get_ptr( "testout.out" );

  outptr.print( "HERE IS THE DATA:\n" );

  //std::ostringstream ss = outptr.get_out_string_stream();
  //ss << xd << " " << yd << " " << zd << " " << xi << " " << yi << " " << zi << " " << xl << " " << yl << " " << zl << std::endl;

  outptr.printf("%lf %lf %lf %d %d %d %ld %ld %ld\n", xd,yd,zd,xi,yi,zi,xl,yl,zl);
  
  outptr.mfile->tofile( ".", outptr.mfile->filename );
  
  
}
