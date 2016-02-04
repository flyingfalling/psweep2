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

#pragma once

#include <filesender.h>
#include <iterator>     // ostream_operator
#include <boost/tokenizer.hpp>

//GET CSV LINE?
std::vector<std::vector<std::string>> parse_CSV_file( const std::string& fname )
{
  std::ifstream in;
  open_ifstream( in, fname.c_str() );
  
  typedef tokenizer< boost::escaped_list_separator<char> > Tokenizer;

  std::vector< std::vector< std::string > > retvec;
  
  std::string line;

  while( getline(in,line) )
    {
      std::vector< std::string > vec;
      Tokenizer tok(line);
      vec.assign(tok.begin(),tok.end());
      retvec.push_back( vec );
    }


  close_ifstream( in );
  
  return retvec;
}

//REV: this is basically growing-matrix...
struct data_table
{
  std::vector< std::string > colnames;
  
  //row-first access I guess.
  std::vector< std::vector< std::string > > dat;
  //Are these guaranteed to be contiguous? Crap...

  //We could do these by "name" as well. Easiest is 0, 0 of course.
  size_t colnum_of_rownames;
  size_t rownum_of_colnames;
  
  
  //REV: Need unique key column? If not, make one?
  data_table( const std::vector< std::vector< std::string> >& tbl, const bool& first_col_head=true )
  {
    if(first_col_head)
      {
	colnames = tbl[0];
	dat = tbl; //REMOVE FIRST ONE? Need to shift them all...fuck...
      }
    else
      {
	colnames.resize( tbl[0].size(), "NOCOLNAME");
      }
    
  }

  //REV: wow, a lot of this stuff is already done by e.g. cv::MAT etc.?!!!
  std::vector< std::string > get_col( const std::string& colname )
  {
    
  }

  std::vector< std::string > get_row( const size_t& rownum )
  {
    
  }

  std::vector< std::string > get_row_by_name( const std::string& rowname )
  {
    
  }

  std::vector< std::string> get_col_by_name( const size_t& colnum )
  {
    
  }

  std::string get_val( const size_t& colnum, const size_t& rownum )
  {
    
  }

  //Return a POD type? I.e. allow them to handle other things than strings after parsing into memory. Force user to specify type?
  //I.e. how to "parse" files. I know what to expect (kind of).
  std::string get_val( const std::string& colname, const std::string& rowname )
  {
    
  }
};


void run_search( const std::string& searchtype, const std::string& scriptfname, const std::string& mydir, const varlist& params )
{
  
  parampoint_generator pg(scriptfname, mydir);
  
  filesender* fs = filesender::Create();
  
  
  if( searchtype.compare( "grid" ) == 0 )
    {
      //Construct required stuff from PARAMS. I.e. min and max of each param? Need N varlists? Have them named? Specific name? Have array type of
      //name PARAMS, etc.? Probably got from a file at beginning... Some special way of reading that...it should know varnames? 
      search_grid( );
    }
  else if( searchtype.compare( "ABC" ) == 0 )
    {
      
    }
  else
    {
      fprintf(stderr, "REV: ERROR, search type [%s] not found\n", searchtype.c_str() );
    }
}


void search_grid( const std::vector<std::string>& varnames,
		  const std::vector<double>& mins,
		  const std::vector<double>& maxes,
		  const std::vector<double>& steps,
		  parampoint_generator& pg,
		  filesender& fs,
		  std::vector<bool>& workingworkers )
{


  //Search_grid will give pg new std::vector<varlist> of vars to send.
  //PROBLEM: Varlists are going to be as varlist<string> (fuck...)
    
  std::vector< std::vector< double > > list_of_permuted_lists;
  std::vector< double > permuted_list_workspace; /* can't do this in parallel I guess... */

  std::vector< std::vector< double > > list_to_permute;
  for(int bin=0; bin<varnames.size(); ++bin)
    {
      std::vector< double > binvect;
      float val=mins[bin];
      while(val < maxes[bin])
	{
	  binvect.push_back(val);
	  val += steps[bin];
	}
      binvect.push_back(val); //assume this is max.
      
      if(val != maxes[bin])
	{
	  fprintf(stdout, "WARNING: PARAM SWEEP GRID: PARAM [%s] STEP DOES NOT EVENLY DIVIDE MIN AND MAX (MIN=%lf MAX=%lf STEP=%lf, but final val is %lf)\n", 
		  varnames[bin].c_str(), mins[bin], maxes[bin], steps[bin], val);
	}
      
      list_to_permute.push_back( binvect );
      
    }
  
  recursive_permute_params(list_to_permute, 0, permuted_list_workspace, list_of_permuted_lists);

  //We now have a list of lists...to run ;) Need to turn each one into a varlist
  std::vector< varlist<std::string> > vls;

  for(size_t x=0; x<list_of_permuted_lists.size(); ++x)
    {
      varlist<std::string> vl;
      vl.add_to_varlist<double>( varnames, list_of_permuted_lists[x] );

      vls.push_back(vl);
    }

  fprintf(stdout, "EXECUTING SEARCH GRID: Now doing COMP PP LIST\n");
  
  //Now, run on vls.
  fs.comp_pp_list(pg, vls, workingworkers);

  fprintf(stdout, "Finished comp PP list, leaving search grid\n");
}
