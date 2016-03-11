#include <csvparser.h>




//GET CSV LINE?
std::vector<std::vector<std::string>> parse_CSV_file( const std::string& fname, const char& sepchar )
{
  std::ifstream in;
  open_ifstream(  fname.c_str() , in);
  
  typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

  boost::escaped_list_separator<char> sep('\\', sepchar, '\"');

  std::vector< std::vector< std::string > > retvec;
  
  std::string line;

  while( getline(in,line) )
    {
      //fprintf(stdout, "Parsing line...[%s]\n", line.c_str());
      std::vector< std::string > vec;
      Tokenizer tok(line, sep);
      vec.assign(tok.begin(),tok.end());
      
      /*fprintf(stdout, "Got vect: [%ld]\n", vec.size());
      for(size_t x=0; x<vec.size(); ++x)
	{
	  fprintf(stdout, " %s", vec[x].c_str());
	}
      fprintf(stdout, "\n");
      */
      retvec.push_back( vec );
    }


  close_ifstream( in );
  
  return retvec;
}

//REV: this is basically growing-matrix...

//Columns must be of regular types? Tagged data types? Need pointers to them. Too complex, someone has done this before damnit.
//Use the other people's stuff.


  void data_table::enumerate()
  {
    for(size_t x=0; x<colnames.size(); ++x)
      {
	fprintf(stdout, "%20s ", colnames[x].c_str() );
      }
    fprintf(stdout, "\n");

    for(size_t x=0; x<nrows; ++x)
      {
	for(size_t y=0; y<ncols; ++y)
	  {
	    fprintf(stdout, "%20s ", get_val( y, x ).c_str()); 
	  }
	fprintf(stdout, "\n");
      }

    fprintf(stdout, "Finished enumerating\n");
  }

  data_table::data_table( const std::string& fname, const bool& hascolnames )
  {
    //Call the other constructor
    char sep = ' '; //REV: do more intelligent, e.g. any spaces.
    construct( parse_CSV_file( fname , sep), hascolnames );
    
    fprintf(stdout, "From file [%s]: CONSTRUCTED DATA TABLE: cols [%ld], rows [%ld]\n", fname.c_str(), ncols, nrows );
    enumerate();
  }
  
  
  
  //REV: Need unique key column? If not, make one?
  void data_table::construct( const std::vector< std::vector< std::string> >& tbl, const bool& first_col_head )
  {

    fprintf(stdout, "data_table ctor, got table of size: nrows [%ld], ncols: [%ld]\n", tbl.size(), tbl[0].size() );
    
    if(first_col_head == true)
      {
	has_colnames = true;
	colnames = tbl[0];
	rownum_of_colnames = 0;

	nrows = tbl.size() - 1;
	ncols = tbl[0].size();

	
	
	for( size_t r=1; r<tbl.size(); ++r)
	  {
	    for(size_t x=0; x<tbl[r].size(); ++x)
	      {
		dat.push_back( tbl[r][x] );
	      }
	  }
      }
    else
      {
	has_colnames = false;
	nrows = tbl.size();
	ncols = tbl[0].size();
	colnames.resize( tbl[0].size(), "NOCOLNAME");

	for( size_t r=0; r<tbl.size(); ++r)
	  {
	    for(size_t x=0; x<tbl[r].size(); ++x)
	      {
		dat.push_back( tbl[r][x] );
	      }
	  }
      }
    
  }

  //REV: wow, a lot of this stuff is already done by e.g. cv::MAT etc.?!!!
  std::vector< std::string > data_table::get_col( const std::string& colname )
  {
    if( !has_colnames )
      {
	fprintf(stdout, "ERROR HAS NO COLUMN NAMES (GET_COL colname)\n");
	exit(1);
      }
    
    std::vector<size_t> locs = find_string_in_vect( colname, colnames );

    if(locs.size() == 0)
      {
	fprintf(stderr, "WHOA, couldn't find colname [%s] in data table, exiting\n", colname.c_str());
	enumerate();
	exit(1);
	
      }
    
    fprintf(stdout, "I think that colname [%s] is colidx [%ld]\n", colname.c_str(), locs[0] );
    
    if( locs.size() != 1 )
      {
	fprintf(stdout, "ERROR MORE THAN ONE OF NAME [%s]\n", colname.c_str());
	exit(1);
      }
    
    size_t colnum = locs[0];
    size_t skip = ncols;
    std::vector< std::string > ret( nrows );
    
    //for( size_t iter=colnum; iter<dat.size(); iter+=skip )
    for( size_t iter=0; iter<nrows; ++iter )
      {
	size_t t= colnum + (ncols * iter);
	ret[iter] = dat[t];
      }
    return ret;
  }

  std::vector< std::string > data_table::get_row( const size_t& rownum )
  {
    
    size_t startloc = rownum * ncols;
    size_t endloc = (rownum * ncols) + ncols;
    std::vector<std::string> ret( dat.begin()+startloc, dat.begin()+endloc );

    return ret;
  }

  std::vector< std::string > data_table::get_row_by_name( const std::string& rowname )
  {
    if( !has_rownames )
      {
	fprintf(stdout, "ERROR HAS NO ROW NAMES\n");
	exit(1);
      }
    
    std::vector<size_t> locs = find_string_in_vect( rowname, rownames );
    
    if( locs.size() != 1 )
      {
	fprintf(stdout, "ERROR MORE THAN ONE OF NAME [%s]\n", rowname.c_str());
	exit(1);
      }

    size_t rownum = locs[0];
    
    size_t startloc = rownum * ncols;
    size_t endloc = (rownum * ncols) + ncols;
    std::vector<std::string> ret( dat.begin()+startloc, dat.begin()+endloc );

    return ret;
  }

  std::vector< std::string> data_table::get_col( const size_t& colnum )
  {
    size_t skip = ncols;
    std::vector< std::string > ret( nrows );

    //REV: is there a better copy iter?
    //for( size_t iter=colnum; iter<dat.size(); iter+=skip )
    for( size_t iter=0; iter<nrows; ++iter )
      {
	size_t t= colnum + (ncols * iter);
	ret[iter] = dat[t];
      }
    return ret;
  }

  std::string data_table::get_val( const size_t& colnum, const size_t& rownum )
  {
    
    size_t loc = rownum*ncols + colnum;
    if( loc >= dat.size() )
      {
	fprintf(stdout, "Get val [%ld] [%ld] is mapping to [%ld]\n", colnum, rownum, loc);
	fprintf(stderr, "ERROR, in get_val, outside of dat array size [%ld]\n", dat.size());
	exit(1);
      }
	  
    return ( dat[loc] );
  }

  //Return a POD type? I.e. allow them to handle other things than strings after parsing into memory. Force user to specify type?
  //I.e. how to "parse" files. I know what to expect (kind of).
  std::string data_table::get_val( const std::string& colname, const std::string& rowname )
  {
    if( !has_rownames || !has_colnames )
      {
	fprintf(stdout, "ERROR HAS NO ROW OR COLUMN NAMES\n");
	exit(1);
      }
    
    std::vector<size_t> clocs = find_string_in_vect( colname, colnames );
    std::vector<size_t> rlocs = find_string_in_vect( rowname, rownames );

    if( clocs.size() != 1 || rlocs.size() != 1)
      {
	fprintf(stdout, "ERROR MORE THAN ONE OF NAME [%s]\n", colname.c_str());
	fprintf(stdout, "ERROR MORE THAN ONE OF NAME [%s]\n", rowname.c_str());
	exit(1);
      }

    size_t colnum = clocs[0];
    size_t rownum = rlocs[0];

    return get_val( colnum, rownum );
  }

