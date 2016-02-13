//REV: header file for searcher, which provides various search functions
//User will call one of these, passing the appropriate inputs,
//and it will automatically construct everything and run the sweep I guess....




//takes user options and runs corresponding search funct?

//"Format" is required...for each different search, what kind of info it needs to parameterize it. I guess just parameterize it with a global varlist.
//REV: OK how to parameterize it with some global varlist?
//Do like PARAMNAME, PARAMMIN, PARAMMAX? How to name it? In individual varlists?
//Use varlists NAMES to find it, inside what? A hierarchical? No...just use a varlist structure... I.e. named list of varlists...Ugh.
//This seems "best", but then user needs to be careful to construct his scripts/etc. as max/min/etc. Or give a better way to read varlists,
//like VAL MIN MAX STEP etc.? So, basically ways of taking 2-d values? Need to take "column" headers, etc. Need it for simulation anyway (read mii-sans
//data?). OK, I guess I can do this. So, now we have some kind of 2d var list structure, where we tell it the variable (row#?) and the column name
// (e.g. var name vs val vs etc.?)
//Effectively we have a 2-column thing now. Make it even more general? We want to eventually have more "abstract" i.e. jagged ones, as we talked about
//before...i.e. nested information.
//I.e. varlists are arbitrary? but then how do we hold arrays? In varlist, it's just an arbitrary list until some endthing? How do I know where it
//ends in read-in? That is the problem?
//Need a way to have arbitrary length array stored somewhere? Only up to 3D array? Either 1D of array type (i.e. all guys are arrays), or 2D of single
//value types...? Need a way to arbitrarily "flatten" matrices of arbitrary length. Ways of storing in memory. Of course, we can but reading/writing
//will not be compressed/efficient. If it includes strings, that will make it only more complex.
//But in the eventuality that I want to run experiments/store it on NSIM side, need a way to do it, right? I don't want to have to repeat every single
//variable every single time. E.g., better to have a way to compress it uniquely. But representation will change if I change number of guys.
//E.g.

//We can rearrange this in some way to make it "hierarchical" based on most efficient graph/tree search... i.e. might be more efficient to "list" it
//by timepoints (var4) if there are more (fewer?) of those. But then how to arbitrarily do it without actually listing out every single one, e.g.
//tell it what each "location" is and then store it that way. I.e. don't write "neuron#" or "time point" each time., just do it? HDF5 might be the
//most elegant way of doing this? But how do I go through and analyse that? E.g. electrode # etc. Doing e.g. EL1 and EL2, or some arbitrary number of
//electrodes (and their positions). How to list those out then? Like, how to specify "experiment" to do most efficiently?
//I.e. TIMING, LOCATION, STRENGTH of each stimulation or something like that. Need a way to parameterize that in an N dimensional space. Timing of
//each is a dimension? Similar to eye movement/visual stimulation analysis. Too many dimensions, and not clear how to orient them to make it most
//informative. Like, I want to basically "list" parameters in some hierarhical way, and finally have e.g. TIMEPOINT and NEURONNUMBER known without
//listing them. Need a "organization" file (how to interpret data file), and then "data file" itself. Same file is obviously best. HDF5?
//Anyway, in this case, do named varlists obviously, easiest. Wasteful. Want to have 2d table things to read too though. Use HDF5...exists, and
//efficient. PRoblem is that stored information might have arbitrary lengths (i.e. strings), or might be doubles or whatever. But to process what
//user is doing, he will make it via some CMD program? Some GUI?

// VAR1 VAR2 VAR3 VAR4 THING
//  1    1     1    1    2.5
//  1    1     1    2    3.5
//  1    1     1    3    95.9
//  1    1     2    1    2.5
//  1    1     2    2    3.5

//For now just pass MIN MAX etc.. Yea, way to handle 2d after all. It contains..? How about going from HDF5 to?
//Hm, this is kind of nasty...because when I "extracted" each column, I would need to appropriately have the return function return
//the kind, or cast it manually each time -_-

#pragma once

#include <filesender.h>
#include <searchalgos.h>

#include <iterator>     // ostream_operator
#include <boost/tokenizer.hpp>
#include <math.h>

#include <utility_functs.h>
//#include <boost/cstdfloat.hpp>
//#include <stdfloat.h>
//#include <stdint.h>

typedef float float32_t;
typedef double float64_t;
typedef int int32_t;
typedef long int int64_t;

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
      fprintf(stdout, "Parsing line...[%s]\n", line.c_str());
      std::vector< std::string > vec;
      Tokenizer tok(line, sep);
      vec.assign(tok.begin(),tok.end());

      fprintf(stdout, "Got vect: [%ld]\n", vec.size());
      for(size_t x=0; x<vec.size(); ++x)
	{
	  fprintf(stdout, " %s", vec[x].c_str());
	}
      fprintf(stdout, "\n");
      retvec.push_back( vec );
    }


  close_ifstream( in );
  
  return retvec;
}

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


  void enumerate()
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

  data_table( const std::string& fname, const bool& hascolnames )
  {
    //Call the other constructor
    char sep = ' '; //REV: do more intelligent, e.g. any spaces.
    construct( parse_CSV_file( fname , sep), hascolnames );
    
    fprintf(stdout, "CONSTRUCTED DATA TABLE: cols [%ld], rows [%ld]\n", ncols, nrows );
    enumerate();
  }
  
  static std::vector<float64_t> to_float64( const std::vector<std::string>& vect )
  {
    std::vector<float64_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	fprintf(stdout, "Double-izing [%s]\n", vect[x].c_str());
	ret[x] = std::stod(vect[x]);
      }
    return ret;
  }

  static std::vector<float32_t> to_float32( const std::vector<std::string>& vect )
  {
    std::vector<float32_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	ret[x] = std::stof(vect[x]);
      }
    return ret;
  }

  static std::vector<int32_t> to_int32( const std::vector<std::string>& vect )
  {
    std::vector<int32_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	ret[x] = std::stoi(vect[x]);
      }
    return ret;
  }

  static std::vector<int64_t> to_int64( const std::vector<std::string>& vect )
  {
    std::vector<int64_t> ret( vect.size() );
    for(size_t x=0; x<vect.size(); ++x)
      {
	ret[x] = std::stol(vect[x]);
      }
    return ret;
  }
  
  //REV: Need unique key column? If not, make one?
  void construct( const std::vector< std::vector< std::string> >& tbl, const bool& first_col_head )
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
  std::vector< std::string > get_col( const std::string& colname )
  {
    if( !has_colnames )
      {
	fprintf(stdout, "ERROR HAS NO COLUMN NAMES (GET_COL colname)\n");
	exit(1);
      }
    
    std::vector<size_t> locs = find_string_in_vect( colname, colnames );

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

  std::vector< std::string > get_row( const size_t& rownum )
  {
    
    size_t startloc = rownum * ncols;
    size_t endloc = (rownum * ncols) + ncols;
    std::vector<std::string> ret( dat.begin()+startloc, dat.begin()+endloc );

    return ret;
  }

  std::vector< std::string > get_row_by_name( const std::string& rowname )
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

  std::vector< std::string> get_col( const size_t& colnum )
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

  std::string get_val( const size_t& colnum, const size_t& rownum )
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
  std::string get_val( const std::string& colname, const std::string& rowname )
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
};


//Globally, I want to have some struct to call it with? Like REGISTER_FUCNT. I don't want user to have to do stuff. I.e. make a "searcher" struct,
//much nicer. Then call run-search. OK.

struct searcher
{

  //Has a fake memsystem in there too hahaha...
  fake_system fakesys;

  void register_fake_funct( const std::string& name, const fake_system_funct_t& funct)
  {
    fakesys.register_funct( name, funct );
  }
  
  //varlist will contain required um, data files I guess?
  void run_search( const std::string& searchtype, const std::string& scriptfname, const std::string& mydir, /*const*/ varlist<std::string>& params )
		   
  {
    fprintf(stdout, "Calling runsearch\n");
    parampoint_generator pg(scriptfname, mydir);
    fprintf(stdout, "made it?\n");

    //REV: PG contains the "results" of each... parampoint_results, of type parampoint_result.
    //That is: list of pset results, each of which has list of pitem results (specifically, varlist).
    //OK, I can access those however I wish, e.g. I know last is the only one I care about etc.

    //REV: User must have created his FAKESYSTEM calls before this point. In other words, in user program, he makes his main, he has his funct,
    //he registers his funct, then when he calls this, he calls it with his list of his FAKE_SYSTEM stuff. OK.
  
    filesender* fs = filesender::Create();
  
    fprintf(stdout, "RUNNING SEARCH ALGO: [%s]\n", searchtype.c_str() );
  
    if( searchtype.compare( "grid" ) == 0 )
      {
      
	std::string varname = "GRID_MIN_MAX_STEP_FILE";
	std::string minmaxfname = params.getTvar( varname );
      
	bool hascolnames = true;
	data_table dtable( minmaxfname, hascolnames );

	fprintf(stdout, "Trying to get VARNAMEs\n");
	std::vector<std::string> varnames = dtable.get_col( "NAME" );

	fprintf(stdout, "Got varnames\n");
	std::vector<double> mins = data_table::to_float64( dtable.get_col( "MIN" ) );
	fprintf(stdout, "Got mins\n");
	std::vector<double> maxes = data_table::to_float64( dtable.get_col( "MAX" ) );
	std::vector<double> steps = data_table::to_float64( dtable.get_col( "STEP" ) );

	fprintf(stdout, "Got STEP\n");
      
	//Construct required stuff from PARAMS. I.e. min and max of each param? Need N varlists? Have them named? Specific name? Have array type of
	//name PARAMS, etc.? Probably got from a file at beginning... Some special way of reading that...it should know varnames? 
	search_grid( varnames, mins, maxes, steps, pg, *fs);
      }
    else if( searchtype.compare( "DREAM-ABC" ) == 0 )
      {
      
      
	//Implement the algorithm...requires quite a bit of messing? Note, what is OUTPUT??? It's the passed PG
      }
    else if( searchtype.compare( "MT-DREAM-zs" ) == 0 )
      {
      
      }
    else
      {
	fprintf(stderr, "REV: ERROR, search type [%s] not found\n", searchtype.c_str() );
      }
  
    fprintf(stderr, "ROOT FINISHED! Broadcasting EXIT\n");
    std::string contents="EXIT";
    boost::mpi::broadcast(fs->world, contents, 0);
  
    delete(fs);
  
  }


}
