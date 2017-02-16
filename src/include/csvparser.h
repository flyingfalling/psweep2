#pragma once

#include <iterator>     // ostream_operator
#include <boost/tokenizer.hpp>
#include <math.h>

#include <commontypes.h>

#include <utility_functs.h>

//GET CSV LINE?
std::vector<std::vector<std::string>> parse_CSV_file( const std::string& fname, const char& sepchar );

//REV: this is basically growing-matrix...

//Columns must be of regular types? Tagged data types? Need pointers to them. Too complex, someone has done this before damnit.
//Use the other people's stuff.

/*
std::vector<size_t> find_string_in_vect( const std::string& targ, const std::vector<std::string>& vect )
{
  std::vector<size_t> locs;
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( targ.compare( vect[x] ) == 0 )
	{
	  locs.push_back( x );
	}
    }

  return locs;
  
}
*/
struct data_table
{
  std::vector< std::string > colnames;
  std::vector< std::string > rownames;
  //row-first access I guess.
  //If different types, we have a problem, because actually stored data may be different size. Note, allocated strings might exist
  //ANYWHERE in memory -_-; Force them all to be doubles?
  //Just do this for now haha.
  //std::vector< std::vector< std::string > > dat;
  std::vector< std::string > dat;
  //Are these guaranteed to be contiguous? Crap...
  
  size_t ncols=666;
  size_t nrows=666;

  bool has_colnames;
  bool has_rownames;
  
  //We could do these by "name" as well. Easiest is 0, 0 of course.
  size_t colnum_of_rownames;
  size_t rownum_of_colnames;


  void enumerate();

  data_table( const std::string& fname, const bool& hascolnames );
  static std::vector<float64_t> /*data_table::*/to_float64( const std::vector<std::string>& vect )
  {
    std::vector<float64_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	//fprintf(stdout, "Double-izing [%s]\n", vect[x].c_str());
	ret[x] = std::stod(vect[x]);
      }
    return ret;
  }

  static std::vector<float32_t> /*data_table::*/to_float32( const std::vector<std::string>& vect )
  {
    std::vector<float32_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	ret[x] = std::stof(vect[x]);
      }
    return ret;
  }

  static std::vector<int32_t> /*data_table::*/to_int32( const std::vector<std::string>& vect )
  {
    std::vector<int32_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	ret[x] = std::stoi(vect[x]);
      }
    return ret;
  }

  static std::vector<int64_t> /*data_table::*/to_int64( const std::vector<std::string>& vect )
  {
    std::vector<int64_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	ret[x] = std::stol(vect[x]);
      }
    return ret;
  }
  /*
  static std::vector<float64_t> to_float64( const std::vector<std::string>& vect );
  
  static std::vector<float32_t> to_float32( const std::vector<std::string>& vect );

  static std::vector<int32_t> to_int32( const std::vector<std::string>& vect );

  static std::vector<int64_t> to_int64( const std::vector<std::string>& vect );
  */
  //REV: Need unique key column? If not, make one?
  void construct( const std::vector< std::vector< std::string> >& tbl, const bool& first_col_head );
 
  //REV: wow, a lot of this stuff is already done by e.g. cv::MAT etc.?!!!
  std::vector< std::string > get_col( const std::string& colname );

  std::vector< std::string > get_row( const size_t& rownum );

  std::vector< std::string > get_row_by_name( const std::string& rowname );

  std::vector< std::string> get_col( const size_t& colnum );

  std::string get_val( const size_t& colnum, const size_t& rownum );

  //Return a POD type? I.e. allow them to handle other things than strings after parsing into memory. Force user to specify type?
  //I.e. how to "parse" files. I know what to expect (kind of).
  std::string get_val( const std::string& colname, const std::string& rowname );
};
