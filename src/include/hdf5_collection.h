//REV: Attributes are not being appropriately written to HDF5 file although they are available in memory after they are created?

//REv: Need to make sure matrices are in their own group (?) otherwise param dataset will be treated as if it is a matrix thing at load time.
//Also the __names thing is problem?

//REV: Make a "backup" function to copy it for safety in case job is killed partway through write?
//Simplest/stupid way is to make sure H5 file is flushed, and then call "copyfile" on the filename...
//To a specific target.

#pragma once

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <utility_functs.h>

#include <H5Cpp.h>
#include <string_manip.h>

#include <vector_utils.h>
#include <commontypes.h>


#define H5TRY() try{

#define H5CATCH() 				\
  } catch (H5::Exception& err)					\
  {								\
    throw std::runtime_error(std::string("HDF5 Error in ")	\
			     + err.getFuncName()		\
			     + ": "				\
			     + err.getDetailMsg());		\
  }

/* std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats); */

/* std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats) */
/* { */
/*   std::vector<std::string> res; */
  
/*   /\* REV: size_t is uint? *\/ */
/*   size_t prev = 0; */
/*   size_t next = 0; */

/*   /\* npos is -1 *\/ */
/*   while ((next = source.find_first_of(delim, prev)) != std::string::npos) */
/*     { */
/*       if (include_empty_repeats || ((next-prev) != 0) ) */
/*         { */
/* 	  res.push_back(source.substr(prev, (next-prev)) ); */
/*         } */
/*       prev = next + 1; */
/*     } */

/*   /\* REV: push back remainder if there is anything left (i.e. after the last token?) *\/ */
/*   if (prev < source.size()) */
/*     { */
/*       res.push_back(source.substr(prev)); */
/*     } */

/*   return res; */
/* } */

H5::DataSpace getSpace( H5::DataSet& ds );

std::vector<std::string> dummy_colnames( const size_t& size );

typedef struct hdf5_dirnames_t
{
  std::vector<std::string> currpath;
  std::vector<std::vector<std::string> > names;
  std::vector<bool> isdir;
} hdf5_dirnames;

herr_t file_info(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata);

herr_t file_info(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata)
{
  //REV: use opdata to communicate "current" working dir as /-separated fname.
  
  //std::vector<std::string> * grpnames = (std::vector<std::string >*)(opdata);
  hdf5_dirnames * grpnames = (hdf5_dirnames*)(opdata);
  
  

  //Lol, apparently depricated but there's no obvious way to switch stuff over to get types/names...ugh.
  //H5Gget_objinfo(loc_id, name, false, &statbuf);
  
  H5O_info_t info;
  herr_t ret = H5Oget_info_by_name(loc_id, name, &info, H5P_DEFAULT);
  
  //REV: why the hell does get_info think everything is a group, but info_by_name works?
  //herr_t ret = H5Oget_info(loc_id, &info);
  hid_t grpid=loc_id;
  //fprintf(stdout, "(grpID=%d)!  ", grpid);
  //OK, so loc_id is what? A kind of label?
  std::vector<std::string> mypath;
  switch (info.type) 
    {
    case H5O_TYPE_GROUP: 
      //fprintf(stdout, " Object with name [%s] is a group (GOING IN!) \n", name);
      grpnames->currpath.push_back( std::string(name) );
      mypath= grpnames->currpath;
      grpnames->names.push_back(mypath); //need a way to mark if it's a dir or not?
      grpnames->isdir.push_back(true); //need a way to mark if it's a dir or not?
      H5Literate_by_name( grpid, name, H5_INDEX_NAME, H5_ITER_INC, NULL, file_info, opdata, H5P_DEFAULT);
      grpnames->currpath.pop_back();
      break;
    case H5O_TYPE_DATASET: 
      //fprintf(stdout, " Object with name [%s] is a dataset \n", name);
      mypath= grpnames->currpath;
      mypath.push_back( std::string(name) );
      grpnames->names.push_back(mypath);
      grpnames->isdir.push_back(false); //need a way to mark if it's a dir or not?
      break;
    case H5O_TYPE_NAMED_DATATYPE: 
      fprintf(stdout, " (WARNING) Object with name [%s] is a named datatype \n", name); //REV: this is just a single basic (atmoic?) type?
      break;
    default:
      fprintf(stdout, " (WARNING) Unable to identify object type (might be H5O_TYPE_NTYPES or H5O_TYPE_UNKNOWN)\n");
      break;
    }
  
  
  return 0;
  
  //hid_t group = H5Gopen2(loc_id, name, H5P_DEFAULT);
  
  //GroupNames.push_back(name);
  //grpnames->push_back( std::string(name) );
  
  //std::cout << "Name : " << name << std::endl;
  
  //H5Gclose(group);
  
  //return 0;
}


std::string build_hdf5_path(std::vector< std::string >& path, std::string basepath="", bool isdir=false);

std::vector<std::string> enumerate_hdf5_dir(H5::H5File& file2, std::string targdir);


std::string build_hdf5_path(std::vector< std::string >& path, std::string basepath, bool isdir);

//I create one of these, and user needs to tell which "variables" he
//wants, like H, Z, Z_new, P, X, etc.. He also tells the "dimensions", all
//of them are allowed to grow?

//Also a list of single variables? Like which generation it is, etc.?

//User does "add named matrix", which might be double etc.
//And then I write to that.

//When I load or start it, make hard links to the matrices so user literally just writes directly through.
//Problem is size must be correct. I.e. accept only vector of correct length. Buffer writes or do it one row at a time?
//Row at a time I guess? or give functions to pass in vectors of vectors? What order will stuff get returned in? We're going to get um,
//varlists from each I guess. Need to make sure they're in the "right order" for writing to file...? I guess alphabetical or something...?
//All columns must have names I guess. String names. Note column names will be stored where? Keep it regularized? Like all must be of same shape?
//Not necessarily true.

struct matrix_props
{
  std::string name; //we use this as name of thing in H5 file.
  H5::DataSet dataset;
  std::vector<std::string> my_colnames;
  size_t my_nrows;
  size_t my_ncols;
  H5::DataType matrix_datatype;

  const std::string __MATRIX_DATATYPE_NAME = "__MY_DATATYPE";
  
  
  std::string get_string_parameter( const std::string& pname );
  
  void add_string_parameter( const std::string& pname, const std::string& val );
  
  //Loads datatype from file.
  /*void load_datatype()
  {
    std::string tmpdatatype = get_string_parameter( __MATRIX_DATATYPE_NAME );
    fprintf(stdout, "LOADED MATRIX (datatype [%s])\n", tmpdatatype.c_str() ) ;
    set_datatype( tmpdatatype );
    }*/
  
  void set_datatype( const std::string& dtype );

  void write_varnames( const std::string& dsetname, const std::vector<std::string>& strings, H5::H5File& f);

  void new_matrix( const std::string& dsetname, const std::vector<std::string>& _colnames, H5::H5File& f, const std::string& datatype );
  
  
  void get_dset_size( size_t& _nrows, size_t& _ncols );
  
  size_t get_ncols();

  size_t get_nrows();

  template <typename T>
  void add_data(  const std::vector<std::vector<T>>& toadd );
  
  template <typename T>
  void add_data( const std::vector<std::string>& colnames, const std::vector<std::vector<T>>& toadd );
  

  template <typename T>
  std::vector< std::vector<T> > read_whole_dataset();
  
  template <typename T>
  std::vector<std::vector< T> > get_last_n_rows( const size_t& nrows );

  
  //REV: This is ****INCLUSIVE***** of end row!!!!!
  template <typename T>
  std::vector< std::vector<T> > read_row_range( const size_t& startrow, const size_t& endrow);

  template <typename T>
  void write_row_range( const size_t& startrow, const size_t& endrow, const std::vector< std::vector<T>>& vals );
  
  //REV: Could manually do this with  datattype...ugh.
  template <typename T>
  void enumerate();
  
  template <typename T>
  void enumerate_to_file( const std::string& fname,  const size_t& thinrate=1, const size_t& startpoint=0 );
  
  
  //Need to do this incrementally incase there is a problem
  void enumerate_to_file( FILE* f, const size_t& skip=1, const size_t& startpoint=0 );
  
  std::vector<std::string> read_string_dset( const std::string& dsname, H5::H5File& f );

  std::vector<std::string> get_varnames() const;
  
  
  //REV: I could make it easier and automatically set datatype but whatever.
  void load_matrix( const std::string& matname, H5::H5File& f );

  //REV: Every **function** will have a type, not the class itself...
};



struct hdf5_collection
{
  H5::H5File file; 
  std::string file_name;
  
  std::vector< matrix_props > matrices;
  std::vector< std::string > parameters; //Just a list of their names. To make backing up easier. Load it when I "load"?
  
  const std::string PARAM_DSET_NAME = "__PARAMETERS";
  const std::string PARAM_GRP_NAME = "__PARAMETERS";
  //const std::string DATA_GRP_NAME = "__DATA";

  bool backup_initialized=false;
  

  void clear();
  
  template <typename T>
  void Xset_numeric_parameter( const std::string& pname, const T& val );
  
  template <typename T>
  void set_numeric_parameter( const std::string& pname, const T& val );

  template <typename T>
  std::vector<T> get_vector_slice( const std::string& pname, const std::vector<size_t>& slices );

  template <typename T>
  std::vector< std::vector<T> > get_matrix_row_slice( const std::string& pname, const std::vector<size_t>& slices );
  
  void Xadd_int64_parameter( const std::string& pname, const int64_t& val );
  

  void add_int64_parameter( const std::string& pname, const int64_t& val );

  void add_float64_parameter( const std::string& pname, const float64_t& val );
  
  void Xadd_float64_parameter( const std::string& pname, const float64_t& val );
  
  void Xadd_string_parameter( const std::string& pname, const std::string& val );

  template <typename T>
  T get_numeric_parameter( const std::string& pname );
  
  template <typename T>
  T Xget_numeric_parameter( const std::string& pname );
  
  std::string Xget_string_parameter( const std::string& pname );

  
  void make_parameters_dataspace();

  void make_parameters_grp();
  
    // either load from memory, or create a new one.
  hdf5_collection(); // const std::string& fname )
 
  void initialize_backup();
  
  void backupCOPY( );
  
  void update_parameter( const std::string& newparam, hdf5_collection& newcol );
  
  
  //Will update targ.
  //Could check other things, but won't for now ;)
  //REV: Can do everything internally?
  void update_matrix( matrix_props& newmat, hdf5_collection& col );
  
  //REV: Could be more intelligent and match them, i.e. take difference...
  //whatever for now.
  void backup_matrices(hdf5_collection& targc);

  void backup_parameters( hdf5_collection& targc );
  
  //Assumes backup file exists (it was copied RAW after first generation! I.e. on creation, we copy it.)
  void backup( );
  
  //Makes a new one as user specifies. Makes empty, creates file etc.
  void new_collection( const std::string& fname );
  
  void add_float64_matrix(const std::string& matname, const std::vector<std::string>& varnames );

  void add_float64_matrix(const std::string& matname, const size_t& ncols );

  void add_int64_matrix(const std::string& matname, const std::vector<std::string>& varnames );

  void add_int64_matrix(const std::string& matname, const size_t& ncols );

  //A 1d vector doesn't have row names?
  void add_float64_vector(const std::string& matname, const std::vector<float64_t>& vals );
  
  //A 1d vector where we might care about the rownames haha.
  void add_float64_vector(const std::string& matname, const std::vector<std::string>& vnames, const std::vector<float64_t>& vals );

  //A 1d vector doesn't have row names?
  void add_int64_vector(const std::string& matname, const std::vector<int64_t>& vals );
  
  template <typename T>
  std::vector<T> get_vector(const std::string& matname )

  
  template <typename T>
  void set_vector(const std::string& matname, const std::vector<T>& vals );
  
  template <typename T>
  void set_vector_element(const std::string& matname, const size_t& targ, const T& val );

  template <typename T>
  T get_vector_element(const std::string& matname, const size_t& targ);
  
  void add_new_matrix( const std::string& matname, const std::vector<std::string>& varnames, const std::string& datatype );

  std::vector<size_t> find_matrix( const std::string& matname );

  
  
  //void load_matrix( const std::string& matname, const std::vector<std::string>& varnames )
  void load_matrix( const std::string& matname );
  

  std::vector<std::string> get_varnames( const std::string& matname );

  
  size_t get_num_rows( const std::string& matname );

  size_t get_num_cols( const std::string& matname );

  template <typename T>
  void add_row_to_matrix( const std::string& matname, const std::vector< T >& vals );

  template<typename T>
  std::vector<std::vector<T> > get_last_n_rows( const std::string& matname, const size_t& nr );

  template<typename T>
  std::vector<T> get_last_row( const std::string& matname );

  
  template <typename T>
  void add_to_matrix( const std::string& matname, const std::vector<std::string>& colnames, const std::vector< std::vector< T > >& vals );

  template <typename T>
  void add_to_matrix( const std::string& matname, const std::vector< std::vector< T > >& vals );
 

  
  template <typename T>
  void write_row_range( const std::string& matname, const size_t& startrow, const size_t& endrow, const std::vector< std::vector< T > >& vals );

  template <typename T>
  void write_row( const std::string& matname, const size_t& startrow, const  std::vector< T > & vals );
  
  template <typename T>
  std::vector<std::vector<T>> read_row_range( const std::string& matname, const size_t& startrow, const size_t& endrow );

  template <typename T>
  std::vector<T> read_row( const std::string& matname, const size_t& row );

  void load_params();
  
  //Opens existing as user specifies. Loads all the matrix_props etc. as user expects.
  void load_collection( const std::string& fname );
  
  std::vector<std::string> matrix_names_from_file();

  template <typename T>
  void enumerate_matrix( const std::string& matname );

  void enumerate_matrix_to_file( const std::string& matname, const std::string& fname,  const size_t& thinrate=1, const size_t& startpoint=0 );
 

  
  
    
};
