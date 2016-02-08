
#pragma once

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <H5Cpp.h>


std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats);

std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats)
{
  std::vector<std::string> res;
  
  /* REV: size_t is uint? */
  size_t prev = 0;
  size_t next = 0;

  /* npos is -1 */
  while ((next = source.find_first_of(delim, prev)) != std::string::npos)
    {
      if (include_empty_repeats || ((next-prev) != 0) )
        {
	  res.push_back(source.substr(prev, (next-prev)) );
        }
      prev = next + 1;
    }

  /* REV: push back remainder if there is anything left (i.e. after the last token?) */
  if (prev < source.size())
    {
      res.push_back(source.substr(prev));
    }

  return res;
}

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

std::vector<std::string> enumerate_hdf5_dir(H5::H5File& file2, std::string targdir)
{
  
  H5::Group rtgrp( file2.openGroup("/") );
    
  hdf5_dirnames names;
  hid_t grpid = rtgrp.getId();
  //fprintf(stdout, "ROOT: Iterating through ID=%d\n", grpid);
  //herr_t idx = H5Literate( rtgrp.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, file_info, (void*)(&names) );
  //herr_t idx = H5Literate( grpid, H5_INDEX_NAME, H5_ITER_INC, NULL, file_info, (void*)(&names) );
  herr_t idx = H5Literate_by_name( grpid, targdir.c_str(), H5_INDEX_NAME, H5_ITER_INC, NULL, file_info, (void*)(&names), H5P_DEFAULT);
  
  
  std::vector<std::string> retvals;
  for(size_t x=0; x<names.names.size(); ++x)
      {
	//fprintf(stdout, "%s\n", build_hdf5_path(names.names[x], names.isdir[x]).c_str());
	retvals.push_back( build_hdf5_path(names.names[x], targdir, names.isdir[x]) );
      }
  
  rtgrp.close();
  
  return retvals;
}


std::string build_hdf5_path(std::vector< std::string >& path, std::string basepath, bool isdir)
{
  //check if end is backslash(s)...and remove.
  char ch = basepath.back(); //only c++11
  std::string workpath=basepath;// + "/"; //just add double backslashes. Solve this problem later.
  if(ch != '/')
    {
      workpath = workpath + "/";
    }
  
  for(size_t x=0; x<path.size(); ++x)
    {
      workpath = workpath + path[x];
      if(x==path.size()-1)
	{
	  if(isdir)
	    {
	      workpath = workpath + "/";
	    }
	  //else, 
	  //     just leave as is.
	}
      else
	{
	  workpath = workpath + "/";
	}
    }
  
  return workpath;
}

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


//We pass names.
//Assume file exists, and is open.  
struct matrix_props
{
  std::string name; //we use this as name of thing in H5 file.
  //size_t nrows;
  //size_t ncols;
  //std::vector<std::string> colnames;

  //ACTUALLY allocated (with fill)
  //USED length (i.e. filled with my data)
    
  H5::DataSet dataset;
  //H5::DataSet colnames_dataset;
  //DataSpace dataspace;

  //Keep around local nrows and ncols, as well as var names.
  std::vector<std::string> my_colnames;

  size_t my_nrows;
  size_t my_ncols;

  void write_varnames( const std::string& dsetname, const std::vector<std::string>& strings, H5::H5File& f)
  {
    H5::Exception::dontPrint();

    try
      {
        // HDF5 only understands vector of char* :-(
        std::vector<const char*> arr_c_str;
        for (size_t ii = 0; ii < strings.size(); ++ii)
	  {
	    arr_c_str.push_back(strings[ii].c_str());
	  }

        //
        //  one dimension
        // 
        hsize_t     str_dimsf[1] {arr_c_str.size()};
        H5::DataSpace   dataspace(1, str_dimsf);

        // Variable length string
        H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE); 
        H5::DataSet str_dataset = f.createDataSet(dsetname, datatype, dataspace);

        str_dataset.write(arr_c_str.data(), datatype);
      }
    catch (H5::Exception& err)
      {
        throw std::runtime_error(std::string("HDF5 Error in ")  
				 + err.getFuncName()
				 + ": "
				 + err.getDetailMsg());
	
	
      }
  }
  
  
  void new_matrix( const std::string& dsetname, const std::vector<std::string>& _colnames, H5::H5File& f )
  {
    name = dsetname;
    my_colnames = _colnames;
    my_ncols = my_colnames.size();
    my_nrows = 0;
    
    const hsize_t ndims = 2;
    const hsize_t ncols = _colnames.size();
    const hsize_t nrows = 0; //start with zero rows.
      
    hsize_t  dims[ndims] = {nrows, ncols};
    hsize_t  max_dims[ndims] = {H5S_UNLIMITED, ncols};

    //Create SPACE
    H5::DataSpace dataspace(ndims, dims, max_dims);
      
    //Create PROPERTIES
    H5::DSetCreatPropList prop;
    const hsize_t nrows_chunk = 100; //Need to mess with CACHE size too!
    hsize_t  chunk_dims[ndims] = { nrows_chunk, ncols};
    prop.setChunk(ndims, chunk_dims);
    double fill_val = -666.666;
    prop.setFillValue( H5::PredType::NATIVE_DOUBLE, &fill_val);
      
    //REV: assume its always native double..ugh. Sometimes I'll write ints though. Just do doubles for now...
    //Need to know type?
    dataset =  f.createDataSet( dsetname, H5::PredType::NATIVE_DOUBLE,
				dataspace, prop) ;

    hsize_t vardim1=_colnames.size();

    std::string col_dname = "__" + dsetname;
    //colnames_dataset = f.createDataSet( col_dname, );
    write_varnames(col_dname, _colnames, f);
    
  }


  //modify dataset also

  void overwrite_data()
  {
      
  }

  void get_dset_size( size_t& _nrows, size_t& _ncols )
  {
    H5::DataSpace origspace = dataset.getSpace();
      
    int rank = origspace.getSimpleExtentNdims();
    //rank should be 2!?!!
    hsize_t dims_out[2];
      
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);
    _nrows = dims_out[0];
    _ncols = dims_out[1];
  }
  
  size_t get_ncols()
  {
    return my_ncols;
  }

  size_t get_nrows()
  {
    return my_nrows;
  }
  
  //Adds one at a time? Really? Ugh.
  //Adds 2d vector, vector of rows basically.

  //Fspace is the hyperslab we selected.
  //Mspace is space targeting just the local data to write.
  void add_data( const std::vector<std::string>& colnames, const std::vector<std::vector<double>>& toadd )
  {
    //Assume that order of colnames is same. Check that for sanity I guess.
    //Need to extend current dataset.
    H5::DataSpace origspace = dataset.getSpace();

    int rank = origspace.getSimpleExtentNdims();
    /*
     * Get the dimension size of each dimension in the dataspace and
     * display them.
     */
    hsize_t dims_out[2];
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);

    //fprintf(stdout, "Got original data space (before extend). [%d] dims: [%lld] row [%lld] col\n", ndims, dims_out[0], dims_out[1] );
      
    if( dims_out[1] != colnames.size() )
      {
	fprintf(stdout, "ERROR, got #cols in hdf5 file datset != expected number trying to add\n");
	exit(1);
      }
      
      
    hsize_t     offset[2] = { dims_out[0], 0 }; //row offset, column offset. Row offset should be equal to current #rows right?
    hsize_t     dims_toadd[2] = { toadd.size(), colnames.size() };
    
    hsize_t size[2] = { dims_out[0]+dims_toadd[0], colnames.size() };
    dataset.extend( size );
      
    //DataSpace targspace2 = dataset.getSpace();
    //REV: get the new (hopefully updated) space...I.e. after extending it.
    origspace = dataset.getSpace(); 

    //Select the hyperslab to write to (in (now exteded) original space ).
    origspace.selectHyperslab( H5S_SELECT_SET, dims_toadd, offset );

    //Define the dataspace of the data to write.
    H5::DataSpace toaddspace( ndims, dims_toadd );

    //REV: Make contiguous data...
    //Row first (i.e. same rows data is grouped) order...

    double vec[toadd.size()][toadd[0].size()];
      
    //std::vector<double> vec( toadd.size() * toadd[0].size() );
    for(size_t x=0; x<toadd.size(); ++x)
      {
	for(size_t y=0; y<toadd[x].size(); ++y )
	  {
	    vec[x][y] = toadd[x][y];
	    //vec[ x*toadd[0].size() + y] = toadd[x][y];
	  }
      }
      
    //dataset.write( toadd.data(), H5::PredType::NATIVE_DOUBLE, toaddspace, origspace );
    //dataset.write( vec.data(), H5::PredType::NATIVE_DOUBLE, toaddspace, origspace );
    dataset.write( vec, H5::PredType::NATIVE_DOUBLE, toaddspace, origspace );
    
    my_nrows += dims_toadd[0];
    
    //fprintf(stdout, "Finished writing\n");
  }

  std::vector< std::vector<double> > read_whole_dataset()
  {
    H5::DataSpace origspace = dataset.getSpace();

    int rank = origspace.getSimpleExtentNdims();
      
    hsize_t dims_out[2];
      
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);
      
    /*if( dims_out[1] != colnames.size() )
      {
      fprintf(stdout, "ERROR, got #cols in hdf5 file datset != expected number trying to add\n");
      exit(1);
      }*/

    std::vector<std::vector<double> > retvect(dims_out[0], std::vector<double>(dims_out[1]) );
    double vec[dims_out[0]][dims_out[1]];
    //dataset.read( retvect.data(), H5::PredType::NATIVE_DOUBLE );
    dataset.read( vec, H5::PredType::NATIVE_DOUBLE );

    for(size_t x=0; x<retvect.size(); ++x)
      {
	for(size_t y=0; y<retvect[x].size(); ++y)
	  {
	    retvect[x][y] = vec[x][y];
	  }
      }
      
    return retvect;
  }
    
  std::vector< std::vector<double> > read_row_range( const size_t& startrow, const size_t& endrow)
  {
      
    //Basically create hyperslab, and then just read that to a correct size local thing.
    H5::DataSpace origspace = dataset.getSpace();
      
    int rank = origspace.getSimpleExtentNdims();
      
    hsize_t dims_out[2];
      
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);

    size_t ncolread = dims_out[1];
      
    if( endrow >= dims_out[0] )
      {
	fprintf(stderr, "SUPER ERROR, trying to read past end of matrix\n");
	exit(1);
      }

    if( endrow < startrow)
      {
	fprintf(stderr, "ERROR, endrow <= startrow [%ld] vs [%ld]\n", endrow, startrow);
	exit(1);
      }
    //If this is zero, we read only 1 row???
    size_t nrowread = endrow-startrow+1; //+1 for reading single row
    double vec[ nrowread ][ ncolread ];

    hsize_t dimsmem[ndims] = {nrowread, ncolread};
      
    //Tells size of vect in mem to write to.
    H5::DataSpace memspace(ndims, dimsmem);
      
    hsize_t offset[ndims] = { startrow, 0 };
      
    origspace.selectHyperslab( H5S_SELECT_SET, dimsmem, offset );
      
    dataset.read( vec, H5::PredType::NATIVE_DOUBLE, memspace, origspace );

    std::vector<std::vector<double>> retvect( nrowread, std::vector<double>(ncolread) );
      
    for(size_t x=0; x<retvect.size(); ++x)
      {
	for(size_t y=0; y<retvect[x].size(); ++y)
	  {
	    retvect[x][y] = vec[x][y];
	  }
      }

    return retvect;
  }
    
  void enumerate()
  {
    fprintf(stdout, "Will enum data set from matrix [%s]:\n\n", name.c_str());
    std::vector<std::vector<double> > ret = read_whole_dataset();
    for(size_t x=0; x<ret.size(); ++x)
      {
	for(size_t y=0; y<ret[x].size(); ++y)
	  {
	    fprintf(stdout, "%5.3f ", ret[x][y]);
	  }
	fprintf(stdout, "\n");
      }
    fprintf(stdout, "\n");
  }
    
  std::vector<std::string> read_string_dset( const std::string& dsname, H5::H5File& f )
  {
    H5::DataSet cdataset = f.openDataSet( dsname );
    

    H5::DataSpace space = cdataset.getSpace();
      
    int rank = space.getSimpleExtentNdims();

    hsize_t dims_out[1];
    
    int ndims = space.getSimpleExtentDims( dims_out, NULL);
    
    size_t length = dims_out[0];

    std::vector<const char*> tmpvect( length, NULL );

    fprintf(stdout, "In read STRING dataset, got number of strings: [%ld]\n", length );

    std::vector<std::string> strs(length);
    H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE); 
    cdataset.read( tmpvect.data(), datatype); // H5::PredType::C_S1 ); //datatype );

    for(size_t x=0; x<tmpvect.size(); ++x)
      {
	fprintf(stdout, "GOT STRING [%s]\n", tmpvect[x] );
	strs[x] = tmpvect[x];
      }
    
    return strs;
  }
  
  //void load_matrix( const std::string& matname, const std::vector<std::string>& varnames, H5::H5File& f )
  void load_matrix( const std::string& matname, H5::H5File& f )
  {

    //REV: Pain in the ass the name will start with root "/". Will it
    //double up?
    std::vector<std::string> tokenized = tokenize_string(matname, "/", false);
    name = tokenized[tokenized.size()-1];
    
    //Load it from the existing file.
    dataset = f.openDataSet( name );

    
    get_dset_size( my_nrows, my_ncols );
    
    //Until we get them saved to HDF5 file...
    //my_colnames = varnames;
    
    //Read, make sure to set properly:
    //VARNAMES, MYNCOLS, MYNROWS, and NAME.
    //REV: Crap, where to store VARNAMES. I guess I need to keep
    //around another dataset which contains the strings...
    //Alternatively, I could rename them? Assume user will have to
    //match them anyway...order will always be same? Always alphebetize
    //them...
    
    //Load from it...
    
    std::string colnamesds = "__" + name;
    //fprintf(stdout, "Will call colnameds with name [%s]?!\n", colnamesds.c_str());
    //colnames_dataset = f.openDataSet( colnamesds );
    my_colnames = read_string_dset( colnamesds, f );
  }
    
  matrix_props() // const std::vector<std::string>& _colnames )
  {
    //REV: Need to have a file created already.
      
    //REV: Need to do chunking etc.
    //Dims always 2. Rows and Columns. Rows first.
      
    //H5Pclose(plist);
    //H5Sclose(file_space);
  }
    
};
  

struct hdf5_collection
{
  //H5std_string file_name; //( "SDS.h5" );

  H5::H5File file; // =NULL; //( FILE_NAME, H5F_ACC_TRUNC );
  
  std::vector< matrix_props > matrices;
  //state. We keep all datasets "open".
  
  //Will either load from memory, or create a new one.
  hdf5_collection() // const std::string& fname )
  {
    
  }
  
  //Makes a new one as user specifies. Makes empty, creates file etc.
  void new_collection( const std::string& fname )
  {
    file = H5::H5File( fname, H5F_ACC_TRUNC );
  }

  void add_new_matrix( const std::string& matname, const std::vector<std::string>& varnames )
  {
    matrix_props mp1;
    mp1.new_matrix(matname, varnames, file);
    matrices.push_back(mp1);
  }

  std::vector<size_t> find_matrix( const std::string& matname )
  {
    std::vector<size_t> ret;
    for(size_t x=0; x<matrices.size(); ++x)
      {
	if( matrices[x].name.compare( matname ) == 0 )
	  {
	    ret.push_back(x);
	  }
      }
    return ret;
  }

  //void load_matrix( const std::string& matname, const std::vector<std::string>& varnames )
  void load_matrix( const std::string& matname )
  {
    matrix_props mp1;
    mp1.load_matrix(matname, file);
    matrices.push_back(mp1);
  }

  size_t get_num_rows( const std::string& matname )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    return matrices[ locs[0] ].get_nrows();
  }

  size_t get_num_cols( const std::string& matname )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    return matrices[ locs[0] ].get_ncols();
  }
  
  
  void add_to_matrix( const std::string& matname, const std::vector<std::string>& colnames, const std::vector< std::vector< double > >& vals )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }

    matrices[ locs[0] ].add_data( colnames, vals );
  }

  std::vector<std::vector<double>> read_row_range( const std::string& matname, const size_t& startrow, const size_t& endrow )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    
    return matrices[ locs[0] ].read_row_range( startrow, endrow );
  }

  std::vector<double> read_row( const std::string& matname, const size_t& row )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    
    return (matrices[ locs[0] ].read_row_range( row, row )[0]);
  }

  //Opens existing as user specifies. Loads all the matrix_props etc. as user expects.
  void load_collection( const std::string& fname )
  {
    file = H5::H5File( fname, H5F_ACC_RDWR );
    //Read all the datasets in there. That requires me enumerating them.
    
    std::vector<std::string> toload = matrix_names_from_file();
    for(size_t x=0; x<toload.size(); ++x)
      {
	if( toload[x][0] != '_' && toload[x][1] != '_' )
	  {
	    load_matrix( toload[x] );
	  }
      }
  }

  std::vector<std::string> matrix_names_from_file()
  {
    std::vector<std::string> datnames = enumerate_hdf5_dir( file, "/" );
    /*for(size_t x=0; x<datnames.size(); ++x)
      {
	fprintf(stdout, "DATASET [%ld]: [%s]\n", x, datnames[x].c_str() );
	}*/
    return datnames;
  }
  void enumerate_matrix( const std::string& matname )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    matrices[ locs[0] ].enumerate();
  }
  
};
