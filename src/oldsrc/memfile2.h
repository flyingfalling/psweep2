
#pragma once

#include <algorithm>                       // copy, min
#include <iosfwd>                          // streamsize
#include <boost/iostreams/categories.hpp>  // source_tag
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>

//REV: use boost iostreams to let user write to a local vector of chars
//as a memory file.

//REV: Or just "get" one from the pointer, i.e. have a mem_ptr which "opens" a file.


/*
//And from: http://www.boost.org/doc/libs/1_44_0/libs/iostreams/doc/classes/array.html
int main()
{
  char buffer[16];
  array_sink sink{buffer};
  stream<array_sink> os{sink};
  os << "Boost" << std::flush;
  std::cout.write(buffer, 5);
}
*/

struct mfile : public boost::iostreams::stream<boost::iostreams::basic_array>
{
  std::vector<char> data;
  boost::iostreams::basic_array ba;
  
 mfile()
   : ba( boost::iostreams::basic_array(data) ),
    boost::iostreams::stream<boost::iostreams::basic_array>( ba )
    {
    }

  void other_funct()
  {
    fprintf(stdout, "Called my funct\n");
  }
};
