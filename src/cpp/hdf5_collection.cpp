
#include <hdf5_collection.h>

//Nothing ;)

H5::DataSpace getSpace( H5::DataSet& ds )
{
  hid_t id2 = ds.getId();
  hid_t myspace = H5Dget_space(id2);
  H5::DataSpace origspace( myspace );
  H5Sclose( myspace );
  return origspace;
}


std::vector<std::string> dummy_colnames( const size_t& size )
{
  return std::vector<std::string>( size, "__NONAME" );
}




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






std::string matrix_props::get_string_parameter( const std::string& pname )
  {
    H5TRY()
      
    H5::Attribute attr = dataset.openAttribute( pname.c_str() ); 
    H5::DataType datatype = attr.getDataType();
         
    std::string strreadbuf("");
    attr.read(datatype, strreadbuf); 
    std::string ret = strreadbuf; 
    return ret;

    H5CATCH()	  

  }

void matrix_props::add_string_parameter( const std::string& pname, const std::string& val )
  {
    H5TRY()

    //REV: MAKE THIS JUST RENAMED? Or is this reading an ID type to initialize it?
    H5::DataSpace attr_dataspace(H5S_SCALAR); // = H5::DataSpace (1, dims );

    H5::StrType datatype(0, H5T_VARIABLE);

    
    H5::Attribute attribute = dataset.createAttribute( pname.c_str(),
						       datatype,
						       attr_dataspace);

    //REV: Crap, I think I need to make sure to always write to a "known" type i.e. in file system space don't use NATIVE_LONG, bc it won't know what
    //it is on other side?
    attribute.write( datatype, val );
    

    H5CATCH()
    return;
  }

void matrix_props::set_datatype( const std::string& dtype )
  {
    //Set attribute to it. Note dataset MUST be loaded beforehand!
    //Set data type to whatever I tell it to, even if it is already set, whatever.
    
    if( dtype.compare( "REAL" ) == 0 )
      {
	fprintf(stdout, "Set data type to REAL\n");
	matrix_datatype = H5::PredType::NATIVE_DOUBLE;
      }
    else if( dtype.compare( "INT" ) == 0 )
      {
	fprintf(stdout, "Set data type to INT\n");
	matrix_datatype = H5::PredType::NATIVE_LONG;
      }
    else
      {
	fprintf(stderr, "REV SUPER ERROR in SET DATATYPE in HDF5 COLLECTION, DATATYPE [%s] not recognize\n", dtype.c_str() );
	exit(1);
      }
  }


void matrix_props::write_varnames( const std::string& dsetname, const std::vector<std::string>& strings, H5::H5File& f)
  {
    //H5::Exception::dontPrint();

    H5TRY()

      // HDF5 only understands vector of char* :-(
    std::vector<const char*> arr_c_str;
    for (size_t i = 0; i < strings.size(); ++i)
      {
	arr_c_str.push_back(strings[i].c_str());
      }

    //
    //  one dimension
    // 
    hsize_t     str_dimsf[1] {arr_c_str.size()};

    //REV: DATASPACE, need to not create a new one as it will consume IDs.
    //This is making a dataspace of 1 RANK and N dims in that rank. (3rd argument is NULL ptr...)
    //I can hack this by doing .setExtentSimple( RANK, DIMSINEACHRANK)
    //dataspace = H5::DataSpace(1, str_dimsf);
    H5::DataSpace dataspace( 1, str_dimsf );

    // Variable length string
    H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE); 
    H5::DataSet str_dataset = f.createDataSet(dsetname, datatype, dataspace);

    str_dataset.write(arr_c_str.data(), datatype);
    //str_dataset.close(); //Automatic at destructor!?
    H5CATCH()
  }


void matrix_props::new_matrix( const std::string& dsetname, const std::vector<std::string>& _colnames, H5::H5File& f, const std::string& datatype )
  {
    
    set_datatype( datatype );
    
    
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
    H5TRY()
    
    H5::DataSpace dataspace(ndims, dims, max_dims);
    //dataspace.setExtentSimple( ndims, dims, max_dims);
    
    //Create PROPERTIES
    H5::DSetCreatPropList prop;
    const hsize_t nrows_chunk = 100; //Need to mess with CACHE size too!
    hsize_t  chunk_dims[ndims] = { nrows_chunk, ncols};
    prop.setChunk(ndims, chunk_dims);

    //T fill_val = 666;// = -666; //lol cheating as it can be either...
    //prop.setFillValue( matrix_datatype, &fill_val);
      
    //REV: assume its always native double..ugh. Sometimes I'll write ints though. Just do doubles for now...
    //Need to know type?
    
    dataset =  f.createDataSet( dsetname, matrix_datatype,
				dataspace, prop);


    
    //REV: SET HERE, because we need dataset created already.
    //add_string_parameter( __MATRIX_DATATYPE_NAME, datatype);
    
    hsize_t vardim1=_colnames.size();
    
    std::string col_dname = "__" + dsetname;

    //colnames_dataset = f.createDataSet( col_dname, );
    write_varnames(col_dname, _colnames, f);
    //prop.close();
    H5CATCH()
  } //end new_matrix


void matrix_props::get_dset_size( size_t& _nrows, size_t& _ncols )
  {
    H5TRY()
      //opendataset();
    //origspace = dataset.getSpace();
    H5::DataSpace origspace = getSpace( dataset );
      
    int rank = origspace.getSimpleExtentNdims();
    //rank should be 2!?!!
    hsize_t dims_out[2];
    
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);
    _nrows = dims_out[0];
    _ncols = dims_out[1];
    //origspace.close();
    //closedataset();
    H5CATCH()
  }

size_t matrix_props::get_ncols()
  {
    return my_ncols;
  }

size_t get_nrows()
  {
    return my_nrows;
  }

 template <typename T>
 void matrix_props::add_data(  const std::vector<std::vector<T>>& toadd )
  {
    add_data<T>( dummy_colnames( toadd[0].size() ), toadd );
  }


template <typename T>
  void matrix_props::add_data( const std::vector<std::string>& colnames, const std::vector<std::vector<T>>& toadd )
  {
    H5TRY()
      //opendataset();
    //Assume that order of colnames is same. Check that for sanity I guess.
    //Need to extend current dataset.
    //origspace = dataset.getSpace();
    H5::DataSpace origspace = getSpace( dataset );
    
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
	fprintf(stderr, "ERROR, got #cols in hdf5 file datset != expected number trying to add (Mat [%s], mat wid [%lld], trying to add [%ld])\n", name.c_str(), dims_out[1], colnames.size() );
	exit(1);
      }
      
      
    hsize_t     offset[2] = { dims_out[0], 0 }; //row offset, column offset. Row offset should be equal to current #rows right?
    hsize_t     dims_toadd[2] = { toadd.size(), colnames.size() };
    
    hsize_t size[2] = { dims_out[0]+dims_toadd[0], colnames.size() };
    dataset.extend( size );


    //origspace.close();
    
    //DataSpace targspace2 = dataset.getSpace();
    //REV: get the new (hopefully updated) space...I.e. after extending it.
    //origspace = dataset.getSpace();
    origspace = getSpace( dataset );

    //Select the hyperslab to write to (in (now exteded) original space ).
    origspace.selectHyperslab( H5S_SELECT_SET, dims_toadd, offset );

    //Define the dataspace of the data to write.
    //toaddspace = H5::DataSpace( ndims, dims_toadd );
    //toaddspace.setExtentSimple(ndims, dims_toadd);
    H5::DataSpace toaddspace(ndims, dims_toadd );

    //REV: Make contiguous data...
    //Row first (i.e. same rows data is grouped) order...

    //T vec[toadd.size()][toadd[0].size()];
    std::vector<T> vec( toadd.size() * toadd[0].size() );
      
    //std::vector<double> vec( toadd.size() * toadd[0].size() );
    //How is it organized in memory? I think it is ROW FIRST for C++. i.e. vec[a][b] will be stored with a iterated through second (obviously).
    //SO bth item will be a=0, b=b.
    for(size_t x=0; x<toadd.size(); ++x)
      {
	for(size_t y=0; y<toadd[x].size(); ++y )
	  {
	    //vec[x][y] = toadd[x][y];
	    //vec[ x*toadd[0].size() + y] = toadd[x][y];
	    vec[ x*toadd[0].size() + y ] = toadd[x][y];
	  }
      }
      
    //dataset.write( toadd.data(), H5::PredType::NATIVE_DOUBLE, toaddspace, origspace );
    //dataset.write( vec.data(), H5::PredType::NATIVE_DOUBLE, toaddspace, origspace );
    //dataset.write( vec.data(), matrix_datatype, toaddspace, origspace );
    H5::DataType mytype = dataset.getDataType();
    dataset.write( vec.data(), mytype, toaddspace, origspace );
    
    my_nrows += dims_toadd[0];
    //closedataset();
    H5CATCH()
    //fprintf(stdout, "Finished writing\n");
  } //end add_data


 template <typename T>
  std::vector< std::vector<T> > matrix_props::read_whole_dataset()
  {
    H5TRY()
      //opendataset();
    H5::DataSpace origspace = getSpace( dataset );

    int rank = origspace.getSimpleExtentNdims();
      
    hsize_t dims_out[2];
      
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);
      
    /*if( dims_out[1] != colnames.size() )
      {
      fprintf(stdout, "ERROR, got #cols in hdf5 file datset != expected number trying to add\n");
      exit(1);
      }*/

    std::vector<std::vector<T> > retvect(dims_out[0], std::vector<T>(dims_out[1]) );
    //T vec[dims_out[0]][dims_out[1]];
    std::vector<T> vec( dims_out[0]*dims_out[1] );
    //dataset.read( retvect.data(), H5::PredType::NATIVE_DOUBLE );
    H5::DataType mytype = dataset.getDataType();
    dataset.read( vec.data(), mytype );

    for(size_t x=0; x<retvect.size(); ++x)
      {
	for(size_t y=0; y<retvect[x].size(); ++y)
	  {
	    //retvect[x][y] = vec[x][y];
	    retvect[x][y] = vec[ x*retvect[x].size() + y ];
	  }
      }
    //closedataset();
    return retvect;
    H5CATCH()
  } //end read_whole_dataset


 template <typename T>
  std::vector<std::vector< T> > matrix_props::get_last_n_rows( const size_t& nrows )
  {
    size_t endrow = my_nrows-1;
    size_t startrow = my_nrows - nrows;
    return read_row_range<T>( startrow, endrow );
  }

//REV: This is ****INCLUSIVE***** of end row!!!!!
  template <typename T>
  std::vector< std::vector<T> > matrix_props::read_row_range( const size_t& startrow, const size_t& endrow)
  {
    
    H5TRY()
      //opendataset();
    //Basically create hyperslab, and then just read that to a correct size local thing.
    //origspace = dataset.getSpace();
    H5::DataSpace origspace = getSpace( dataset );

    //fprintf(stdout, "ORIGSPACE ID is: [%ld]\n", origspace.getId());
      
    int rank = origspace.getSimpleExtentNdims();
      
    hsize_t dims_out[2];

    //origspace.close();
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);

    hsize_t ncolread = dims_out[1];
      
    if( endrow >= dims_out[0] )
      {
	
	fprintf(stderr, "SUPER ERROR (MATRIX [%s]), trying to read past end of matrix (requested endrow [%ld], but matrix size is [%lld])\n", name.c_str(), endrow, dims_out[0]);
	exit(1);
      }

    if( endrow < startrow)
      {
	fprintf(stderr, "ERROR, READROWRANGE: endrow <= startrow [%ld] vs [%ld] (matrix is [%s])\n", endrow, startrow, name.c_str());
	exit(1);
      }
    //If this is zero, we read only 1 row???
    hsize_t nrowread = endrow-startrow+1; //+1 for reading single row

    //T vec[ nrowread ][ ncolread ];
    //Can I make it contiguous? I guess so...
    std::vector< T > vec( nrowread * ncolread ); //Make it into single contiguous memory?
    //Problem was that vec was being stack-allocated, which caused a segfualt...

    hsize_t dimsmem[ndims] = {nrowread, ncolread};
      
    //Tells size of vect in mem to write to.
    //memspace=H5::DataSpace(ndims, dimsmem);
    //memspace.setExtentSimple(ndims, dimsmem);
    H5::DataSpace memspace(ndims, dimsmem);
    
    //fprintf(stdout, "MEMSPACE Id is [%ld]\n", memspace.getId());
    
    hsize_t offset[ndims] = { startrow, 0 };
    
    origspace.selectHyperslab( H5S_SELECT_SET, dimsmem, offset );

    H5::DataType mytype = dataset.getDataType();
    dataset.read( vec.data(), mytype, memspace, origspace );
    
    std::vector<std::vector<T>> retvect( nrowread, std::vector<T>(ncolread) );
    
    for(size_t x=0; x<retvect.size(); ++x)
      {
	for(size_t y=0; y<retvect[x].size(); ++y)
	  {
	    //retvect[x][y] = vec[x][y];
	    retvect[x][y] = vec[ x*retvect[x].size() + y ];
	  }
      }

    //closedataset();
    return retvect;
    H5CATCH()
  } //end read_row_range


template <typename T>
  void matrix_props::write_row_range( const size_t& startrow, const size_t& endrow, const std::vector< std::vector<T>>& vals )
  {

    H5TRY()
      //opendataset();
    //Basically create hyperslab, and then just read that to a correct size local thing.
    //origspace = dataset.getSpace();
    H5::DataSpace origspace = getSpace(dataset);
      
    int rank = origspace.getSimpleExtentNdims();
      
    hsize_t dims_out[2];
      
    int ndims = origspace.getSimpleExtentDims( dims_out, NULL);

    size_t ncolwrite = dims_out[1];
      
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
    size_t nrowwrite = endrow-startrow+1; //+1 for reading single row

    if(vals.size() != nrowwrite )
      {
	fprintf(stderr, "ERROR in matrix write: passed value array does not fill specified start/end rows!\n");
	exit(1);
      }
    //T vec[ nrowwrite ][ ncolwrite ];
    std::vector<T> vec(nrowwrite*ncolwrite);
    
    //REV: Fill vec, haha copying...oh well.
    for(size_t x=0; x<vals.size(); ++x)
      {
	for(size_t y=0; y<vals[x].size(); ++y)
	  {
	    //vec[x][y] = vals[x][y];
	    vec[ x*vals[x].size() + y ] = vals[x][y];
	  }
      }
    
    hsize_t dimsmem[ndims] = {nrowwrite, ncolwrite};
    
    //Tells size of vect in mem to write to.
    H5::DataSpace memspace( ndims, dimsmem);
    
    hsize_t offset[ndims] = { startrow, 0 };
    
    origspace.selectHyperslab( H5S_SELECT_SET, dimsmem, offset );

    H5::DataType mytype = dataset.getDataType();
    dataset.write( vec.data(), mytype, memspace, origspace );
    //closedataset();
    H5CATCH()
    return;
  } //end read_row_range


template <typename T>
  void matrix_props::enumerate()
  {
    fprintf(stdout, "Will enum data set from matrix [%s]:\n\n", name.c_str());
    std::vector<std::vector<T> > ret = read_whole_dataset<T>();
    for(size_t x=0; x<ret.size(); ++x)
      {
	for(size_t y=0; y<ret[x].size(); ++y)
	  {
	    std::cout << ret[x][y] << " ";
	    //fprintf(stdout, "%lf ", ret[x][y]);
	  }
	fprintf(stdout, "\n");
      }
    fprintf(stdout, "\n");
  }


template <typename T>
  void matrix_props::enumerate_to_file( const std::string& fname,  const size_t& thinrate=1, const size_t& startpoint=0 )
  {
    fprintf(stdout, "Will enum data set from matrix [%s]:\n\n", name.c_str());

    std::ofstream outfile;
    //Non binary...
    open_ofstream( fname, outfile, std::ios_base::trunc );
    
    //REV: This may destroy memory if it's too big...
    std::vector<std::vector<T> > ret = read_whole_dataset<T>();
    for(size_t x=startpoint; x<ret.size(); x+=thinrate)
      {
	if(ret[x].size() == 0)
	  {
	    fprintf(stderr, "ERROR in enumerate to file: no columns?!\n");
	    exit(1);
	  }
	
	outfile << ret[x][0];
	for(size_t y=1; y<ret[x].size(); ++y)
	  {
	    outfile << " " << ret[x][y];
	    //fprintf(stdout, "%lf ", ret[x][y]);
	  }
	outfile << std::endl;
      }
    outfile << std::endl;
  }

//Need to do this incrementally incase there is a problem
  void matrix_props::enumerate_to_file( FILE* f, const size_t& skip=1, const size_t& startpoint=0 )
  {
    for(size_t x=0; x<my_colnames.size(); ++x)
      {
	fprintf(f, "%s ", my_colnames[x].c_str() );
      }
    fprintf(f, "\n");

    H5::DataType mytype = dataset.getDataType();
    
    if( mytype == H5::PredType::NATIVE_DOUBLE )
      {
	std::vector<std::vector<float64_t> > ret = read_whole_dataset<float64_t>();
	for( size_t x=startpoint; x<ret.size(); x+=skip )
	  {
	    fprintf(f, "%lf", ret[x][0]);
	    for(size_t y=1; y<ret[x].size(); ++y)
	      {
		fprintf(f, " %lf", ret[x][y]);
	      }
	    fprintf(f, "\n");
	  }
      }
    else
      {
	std::vector<std::vector<int64_t> > ret = read_whole_dataset<int64_t>();
	for( size_t x=startpoint; x<ret.size(); x+=skip )
	  {
	    fprintf(f, "%ld", ret[x][0]);
	    for(size_t y=1; y<ret[x].size(); ++y)
	      {
		fprintf(f, " %ld", ret[x][y]);
	      }
	    fprintf(f, "\n");
	  }
      }

  } //end enumerate_to_file


 std::vector<std::string> matrix_props::read_string_dset( const std::string& dsname, H5::H5File& f )
  {
    H5TRY()

    H5::DataSet cdataset = f.openDataSet( dsname );
    
    
    H5::DataSpace space = getSpace( cdataset );
      
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
    //space.close();
    return strs;
    H5CATCH()
  } //read_string_dset


std::vector<std::string> matrix_props::get_varnames() const
  {
    return my_colnames;
  }


//REV: I could make it easier and automatically set datatype but whatever.
void matrix_props::load_matrix( const std::string& matname, H5::H5File& f ) //, const std::string& datatype )
  {
    H5TRY()
    
    //REV: Pain in the ass the name will start with root "/". Will it
    //double up?
    std::vector<std::string> tokenized = tokenize_string(matname, "/", false);
    name = tokenized[tokenized.size()-1];
    fprintf(stdout, "In load matrix: will try to load name [%s]\n", name.c_str());
    
    //Load it from the existing file.
    
    //REV CHECKING IF ITS DSET OPEN/CLOSE that matters!!
    dataset = f.openDataSet( name );
    
    //I don't need TYPE to open the dataset (yet)
    //load_datatype();
    
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
    
    H5CATCH()
  }







void hdf5_collection::clear()
  {
    file.close();
    file_name="";
    matrices.clear();
    parameters.clear();
    backup_initialized=false;
  }

template <typename T>
  void hdf5_collection::Xset_numeric_parameter( const std::string& pname, const T& val )
  {
    H5::DataSet dataset = file.openDataSet( PARAM_DSET_NAME );

    //REV: USe GETDATATYPE (should return H5::PredType::BLAH!?!?!). Great! So only set data type at creation heh.
    H5::Attribute attr = dataset.openAttribute( pname.c_str() );
    H5::DataType type = attr.getDataType();
    attr.write(type, &val);

    //int i=1;
    //dataset.write( &i, H5::PredType::NATIVE_INT );

    //REV: AH crap, need to write the dataset or the parameter will not be written to the file? Or is it because the dataset is empty.
    return;
  }

template <typename T>
  void hdf5_collection::set_numeric_parameter( const std::string& pname, const T& val )
  {
    H5::Group group = file.openGroup( PARAM_GRP_NAME );

    //REV: USe GETDATATYPE (should return H5::PredType::BLAH!?!?!). Great! So only set data type at creation heh.
    H5::DataSet ds = group.openDataSet( pname );
    H5::DataSpace dataspace = getSpace( ds );
    H5::DataType type = ds.getDataType();
    ds.write(&val, type, dataspace);

    //int i=1;
    //dataset.write( &i, H5::PredType::NATIVE_INT );

    //REV: AH crap, need to write the dataset or the parameter will not be written to the file? Or is it because the dataset is empty.
    return;
  }

template <typename T>
  std::vector<T> hdf5_collection::get_vector_slice( const std::string& pname, const std::vector<size_t>& slices )
  {
    std::vector<T> ret( slices.size() ); //better not be any repeats in slices...?
    for(size_t x=0; x<slices.size(); ++x)
      {
	//Can optimize...by only reading it once...do it later
	ret[x] = get_numeric_parameter<T>( pname, slices[x] );
      }
    return ret;
  }

template <typename T>
  std::vector< std::vector<T> > hdf5_collection::get_matrix_row_slice( const std::string& pname, const std::vector<size_t>& slices )
  {
    std::vector< std::vector<T> > ret( slices.size(), std::vector<T>(get_num_cols( pname ) ) ); //better not be any repeats in slices...?
    for(size_t x=0; x<slices.size(); ++x)
      {
	//Can optimize...by opening matrix only once...do later.
	//std::vector<T> r = read_row( pname, slices[x] );
	//ret.push_back( r );
	ret[x] = read_row<T>( pname, slices[x] );
      }
    return ret;
  }

void hdf5_collection::Xadd_int64_parameter( const std::string& pname, const int64_t& val )
  {
    hsize_t numdims=1;
    hsize_t DIM1length=1;
    hsize_t dims[numdims] = { DIM1length };
    H5::DataSpace attr_dataspace(1, dims );
    
    H5::DataSet dataset = file.openDataSet( PARAM_DSET_NAME );
    H5::Attribute attribute = dataset.createAttribute( pname.c_str(),
						   H5::PredType::NATIVE_LONG, 
						   attr_dataspace);

    //REV: Crap, I think I need to make sure to always write to a "known" type i.e. in file system space don't use NATIVE_LONG, bc it won't know what
    //it is on other side?
    attribute.write( H5::PredType::NATIVE_LONG, &val );
    
    parameters.push_back( pname );
    
    return;
  }

void hdf5_collection::add_int64_parameter( const std::string& pname, const int64_t& val )
  {
    hsize_t numdims=1;
    hsize_t DIM1length=1;
    hsize_t dims[numdims] = { DIM1length };
    H5::DataSpace dataspace(1, dims );

    H5::Group group = file.openGroup( PARAM_GRP_NAME );
    H5::DataSet dataset = group.createDataSet( pname, H5::PredType::NATIVE_LONG, dataspace  ); //REV: Need properties?
    
    dataset.write(  &val, H5::PredType::NATIVE_LONG, dataspace );
    
    parameters.push_back( pname );
    
    return;
  }


void hdf5_collection::add_float64_parameter( const std::string& pname, const float64_t& val )
  {
    hsize_t numdims=1;
    hsize_t DIM1length=1;
    hsize_t dims[numdims] = { DIM1length };
    H5::DataSpace dataspace(1, dims );

    H5::Group group = file.openGroup( PARAM_GRP_NAME );
    H5::DataSet dataset = group.createDataSet( pname, H5::PredType::NATIVE_DOUBLE, dataspace  ); //REV: Need properties?
    
    dataset.write(  &val, H5::PredType::NATIVE_DOUBLE, dataspace );
    
    parameters.push_back( pname );
    
    return;
  }
  
  void hdf5_collection::Xadd_float64_parameter( const std::string& pname, const float64_t& val )
  {
    hsize_t numdims=1;
    hsize_t DIM1length=1;
    hsize_t dims[numdims] = { DIM1length };
    H5::DataSpace attr_dataspace(1, dims );
    
    H5::DataSet dataset = file.openDataSet( PARAM_DSET_NAME );
    H5::Attribute attribute = dataset.createAttribute( pname.c_str(),
						       /*H5::PredType::IEEE_F64BE*/ H5::PredType::NATIVE_DOUBLE , 
						   attr_dataspace);

    //REV: Crap, I think I need to make sure to always write to a "known" type i.e. in file system space don't use NATIVE_LONG, bc it won't know what
    //it is on other side?
    attribute.write( H5::PredType::NATIVE_DOUBLE, &val );

    parameters.push_back( pname );
    
    return;
  }


 void hdf5_collection::Xadd_string_parameter( const std::string& pname, const std::string& val )
  {
    //hsize_t numdims=1;
    //hsize_t DIM1length=1;
    //hsize_t dims[numdims] = { DIM1length };
    //REV: This is OK I think?
    H5::DataSpace attr_dataspace2(H5S_SCALAR); // = H5::DataSpace (1, dims );

    H5::StrType datatype(0, H5T_VARIABLE);
    
    H5::DataSet dataset = file.openDataSet( PARAM_DSET_NAME );
    H5::Attribute attribute = dataset.createAttribute( pname.c_str(),
						       datatype,
						       attr_dataspace2);

    //REV: Crap, I think I need to make sure to always write to a "known" type i.e. in file system space don't use NATIVE_LONG, bc it won't know what
    //it is on other side?
    attribute.write( datatype, val );

    parameters.push_back( pname );
    
    return;
  }

  template <typename T>
  T hdf5_collection::get_numeric_parameter( const std::string& pname )
  {
    H5::Group group = file.openGroup( PARAM_GRP_NAME );
    H5::DataSet ds = group.openDataSet( pname );
    H5::DataSpace dataspace = getSpace( ds );
    H5::DataType type = ds.getDataType();
    
    T ret;
    
    ds.read(&ret, type, dataspace );
    
    return ret;
  }
  
  template <typename T>
  T hdf5_collection::Xget_numeric_parameter( const std::string& pname )
  {
    H5::DataSet dataset = file.openDataSet( PARAM_DSET_NAME );
    // Open attribute and read its contents
    //hsize_t numdims=1;
    //hsize_t DIM1length=1;
    //hsize_t dims[numdims] = { DIM1length };
    //DataSpace attr_dataspace = DataSpace (1, dims );

    T ret;
    
    H5::Attribute attr = dataset.openAttribute( pname.c_str() );
    H5::DataType type = attr.getDataType();
    attr.read(type, &ret);

    return ret;
  }
  
  std::string hdf5_collection::Xget_string_parameter( const std::string& pname )
  {
    // Set up read buffer for attribute
    

    H5::DataSet dataset = file.openDataSet( PARAM_DSET_NAME );
        
    H5::Attribute attr = dataset.openAttribute( pname.c_str() );
    H5::DataType datatype = attr.getDataType();
    
    std::string strreadbuf("");
    attr.read(datatype, strreadbuf);

    std::string ret = strreadbuf;
    return ret;

  }


void hdf5_collection::make_parameters_dataspace()
  {
    hsize_t  ndims=1;
    hsize_t  dims[ndims];
    dims[0] = 1;
    
    
    H5::DataSpace dataspace(ndims, dims);
    //dataspace.setExtentSimple(ndims, dims);
    H5::DSetCreatPropList ds_creatplist;
    file.createDataSet( PARAM_DSET_NAME, H5::PredType::NATIVE_INT,
			dataspace, ds_creatplist  );
  }

  void hdf5_collection::make_parameters_grp()
  {
    file.createGroup( PARAM_GRP_NAME ); //Don't need to write yet.
  }
  
    // either load from memory, or create a new one.
  hdf5_collection::hdf5_collection() // const std::string& fname )
  {
    
  }

  void hdf5_collection::initialize_backup()
  {
    backupCOPY(); //performs raw copy. Should not be larger than max size...?
    backup_initialized=true;
  }
  
  void hdf5_collection::backupCOPY( )
  {
    //Automatically backups to "__"+file_name
    std::string bufname = "__" + file_name;

    file.flush(H5F_SCOPE_GLOBAL);
    
    fprintf(stdout, "Copying (backing up) file [%s] to [%s]\n", file_name.c_str(), bufname.c_str() );

    //REV: Sendfile requires file size be < 2GB!!!!!!!!!
    copy_file( file_name, bufname );
    return;
  }
  
  void hdf5_collection::update_parameter( const std::string& newparam, hdf5_collection& newcol )
  {
    H5::Group grp = file.openGroup( PARAM_GRP_NAME );
    H5::DataSet ds = grp.openDataSet( newparam );
    H5::DataType type = ds.getDataType();

    if(type == H5::PredType::NATIVE_DOUBLE )
      {
	float64_t v = get_numeric_parameter<float64_t>( newparam );
	newcol.set_numeric_parameter<float64_t>( newparam, v );
      }
    else if (type == H5::PredType::NATIVE_LONG )
      {
	float64_t v = get_numeric_parameter<int64_t>( newparam );
	newcol.set_numeric_parameter<int64_t>( newparam, v );
      }
    else
      {
	fprintf(stderr, "ERROR in update_param in backup, unknown type\n");
	exit(1);
      }
  }


//Will update targ.
  //Could check other things, but won't for now ;)
  //REV: Can do everything internally?
  void hdf5_collection::update_matrix( matrix_props& newmat, hdf5_collection& col )
  {
    std::vector<size_t> locs = col.find_matrix( newmat.name );
    if(locs.size() != 1 )
      {
	fprintf(stderr, "Could not find matrix [%s] in target backup collection!\n", newmat.name.c_str() );
	exit(1);
      }
    size_t i = locs[0];
    //What the hell make a copy of it why not? Lol... problem is when the
    //other guy exits it might cause a problem? At cleanup time?
    
    std::string targmatname = newmat.name;
    if( newmat.get_nrows() > col.matrices[i].get_nrows() )
      {
	//size_t rowsdiff = newmat.get_nrows() - targmat.get_nrows();
	size_t rowsdiff = newmat.get_nrows() - col.matrices[i].get_nrows();
	H5::DataType mytype = col.matrices[i].dataset.getDataType();
	if( mytype == H5::PredType::NATIVE_DOUBLE)
	  {
	    std::vector< std::vector< float64_t > > toadd =
	      newmat.get_last_n_rows<float64_t>( rowsdiff );

	    col.matrices[i].add_data<float64_t>( toadd );
	  }
	else if(mytype == H5::PredType::NATIVE_LONG)
	  {
	    std::vector< std::vector< int64_t > > toadd =
	      newmat.get_last_n_rows<int64_t>( rowsdiff );
	    
	    col.matrices[i].add_data<int64_t>( toadd );
	  }
	else
	  {
	    fprintf(stderr, "ERROR, unknown type in update_matrix for backup\n");
	    exit(1);
	  }
	//Need to update by adding rows. NEED TO KNOW TYPE
      }
    else if ( newmat.get_nrows() == col.matrices[i].get_nrows() )
      {
	return;
	//we assume in no case is a vector updated without adding rows.
	//I.e. there are no pure "state" vectors!
	//We could do a check for #rows==1 and do it that way...?
      }
    else
      {
	fprintf(stderr, "ERROR, backup has MORE rows?!?!?!!\n");
	exit(1);
      }
  }
  
  //REV: Could be more intelligent and match them, i.e. take difference...
  //whatever for now.
  void hdf5_collection::backup_matrices(hdf5_collection& targc)
  {
    if( matrices.size() != targc.matrices.size() )
      {
	fprintf(stderr, "WHOA in backup matrices! Not same number of matrices of source file: [%s] (%ld) and target [%s] (%ld)\n", file_name.c_str(), matrices.size(), targc.file_name.c_str(), targc.matrices.size() );
	for(size_t x=0; x<matrices.size(); ++x)
	  {
	    fprintf(stderr, "ORIG: [%s]\n", matrices[x].name.c_str() );
	  }
	for(size_t x=0; x<targc.matrices.size(); ++x)
	  {
	    fprintf(stderr, "TARG: [%s]\n", targc.matrices[x].name.c_str() );
	  }
	exit(1);
      }
    for(size_t m=0; m<matrices.size(); ++m)
      {
	update_matrix( matrices[m], targc );
      }
  }

void hdf5_collection::backup_parameters( hdf5_collection& targc )
  {
    if( parameters.size() != targc.parameters.size() )
      {
	fprintf(stderr, "WHOA in backup parameters! Not same number of parameters of source file: [%s] (%ld) and target [%s] (%ld)\n", file_name.c_str(), parameters.size(), targc.file_name.c_str(), targc.parameters.size() );
	exit(1);
      }
    for(size_t m=0; m<parameters.size(); ++m)
      {
	update_parameter( parameters[m], targc );
      }
  }
  
  //Assumes backup file exists (it was copied RAW after first generation! I.e. on creation, we copy it.)
  void hdf5_collection::backup( )
  {
    //REV: This makes it so they match more...?
    file.flush(H5F_SCOPE_GLOBAL);

    fprintf(stderr, "Calling backup\n");
    if(!backup_initialized)
      {
	//REV: PROBLEM the copy takes some time initially (it's not blocking until filesystem copy is done?!?!)
	initialize_backup();
	return;
      }
    
    //Automatically backups to "__"+file_name
    std::string bufname = "__" + file_name;
    
    //REV: More efficient method to backup (and gets rid of 2GB max file size from sendfile() haha...damn that's stupid.
    //Only copies DIFFERENCES. If matrix is LARGER, it only copies the difference in rows at the end.
    //For all parameters, it copies them.
    fprintf(stdout, "Copying (backing up) file [%s] to [%s]\n", file_name.c_str(), bufname.c_str() );

    hdf5_collection tmpc;
    tmpc.load_collection( bufname );

    //We can now do sets etc. based on diffs.

    //H5::H5File tmpfile( bufname,  );
    backup_matrices( tmpc );
    backup_parameters( tmpc );

    //Flush the file of the tmp guy and hope it's blocking.
    
    return;
  }




 //Makes a new one as user specifies. Makes empty, creates file etc.
  void hdf5_collection::new_collection( const std::string& fname )
  {
    file = H5::H5File( fname, H5F_ACC_TRUNC );
    file_name = fname;
    make_parameters_grp();
    //make_parameters_dataspace();
  }
  
  void hdf5_collection::add_float64_matrix(const std::string& matname, const std::vector<std::string>& varnames )
  {
    add_new_matrix( matname, varnames, "REAL");
  }

  void hdf5_collection::add_float64_matrix(const std::string& matname, const size_t& ncols )
  {
    add_new_matrix( matname, dummy_colnames(ncols), "REAL");
  }

  void hdf5_collection::add_int64_matrix(const std::string& matname, const std::vector<std::string>& varnames )
  {
    add_new_matrix( matname, varnames, "INT");
  }

  void hdf5_collection::add_int64_matrix(const std::string& matname, const size_t& ncols )
  {
    add_new_matrix( matname, dummy_colnames(ncols), "INT");
  }

  //A 1d vector doesn't have row names?
  void hdf5_collection::add_float64_vector(const std::string& matname, const std::vector<float64_t>& vals )
  {
    add_float64_matrix( matname, dummy_colnames( vals.size() ) );
    add_row_to_matrix( matname, vals );
  }

  //A 1d vector where we might care about the rownames haha.
  void hdf5_collection::add_float64_vector(const std::string& matname, const std::vector<std::string>& vnames, const std::vector<float64_t>& vals )
  {
    add_float64_matrix( matname, vnames );
    add_row_to_matrix( matname, vals );
  }

  //A 1d vector doesn't have row names?
  void hdf5_collection::add_int64_vector(const std::string& matname, const std::vector<int64_t>& vals )
  {
    add_int64_matrix( matname, dummy_colnames( vals.size() ) );
    add_row_to_matrix( matname, vals );
  }
  
  template <typename T>
  std::vector<T> hdf5_collection::get_vector(const std::string& matname )
  {
    return read_row<T>( matname, 0 );
  }
  
  template <typename T>
  void hdf5_collection::set_vector(const std::string& matname, const std::vector<T>& vals )
  {
    //write row 0 to vals
    write_row<T>( matname, 0, vals );
    return;
  }
  
  template <typename T>
  void hdf5_collection::set_vector_element(const std::string& matname, const size_t& targ, const T& val )
  {
    std::vector<T> ret = get_vector<T>(matname);
    //Make sure it's not out of bounds etc.
    ret[targ]=val;
    set_vector( matname, ret );
    return;
  }

  template <typename T>
  T hdf5_collection::get_vector_element(const std::string& matname, const size_t& targ)
  {
    std::vector<T> ret = get_vector<T>(matname);
    return ret[targ];
  }












  void hdf5_collection::add_new_matrix( const std::string& matname, const std::vector<std::string>& varnames, const std::string& datatype )
  {
    matrix_props mp1;
    mp1.new_matrix(matname, varnames, file, datatype );
    matrices.push_back(mp1);
  }

  std::vector<size_t> hdf5_collection::find_matrix( const std::string& matname )
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
  void hdf5_collection::load_matrix( const std::string& matname ) //, const std::string& datatype )
  {
    matrix_props mp1;
    mp1.load_matrix(matname, file );
    matrices.push_back(mp1);
  }

  std::vector<std::string> get_varnames( const std::string& matname )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size( ) );
      }
    return matrices[ locs[0] ].get_varnames();
  }

  
  size_t hdf5_collection::get_num_rows( const std::string& matname )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size( ) );
      }
    return matrices[ locs[0] ].get_nrows();
  }

  size_t hdf5_collection::get_num_cols( const std::string& matname )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    return matrices[ locs[0] ].get_ncols();
  }

  template <typename T>
  void hdf5_collection::add_row_to_matrix( const std::string& matname, const std::vector< T >& vals )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    
    std::vector< std::vector<T> > f;
    f.push_back( vals );
    add_to_matrix( matname, dummy_colnames( vals.size() ), f );
    return;
  }

  template<typename T>
  std::vector<std::vector<T> > hdf5_collection::get_last_n_rows( const std::string& matname, const size_t& nr )
  {
    if( nr == 0 )
      {
	fprintf(stderr, "ERROR: trying to get ZERO rows from last N rows...\n");
	exit(1);
      }
    size_t len = get_num_rows( matname );
    if( len < nr )
      {
	fprintf(stderr, "REV: ERROR in get_last_n_rows: not enough rows...( mat [%s], trying to get [%ld] rows but only [%ld] exist)\n", matname.c_str(), nr, len );
	exit(1);
      }
    size_t startrow = len-nr;
    size_t endrow = len-1; //Reads "include" the end row!!!!
    return read_row_range<T>( matname, startrow, endrow );
  }

  template<typename T>
  std::vector<T> hdf5_collection::get_last_row( const std::string& matname )
  {
    size_t len = get_num_rows( matname );
    if( len < 1 )
      {
	fprintf(stderr, "REV: ERROR in get_last_n_rows: not enough rows...( mat [%s], trying to get [%d] rows but only [%ld] exist)\n", matname.c_str(), 1, len );
	exit(1);
      }
    size_t startrow = len-1;
    //size_t endrow = len-1; //Reads "include" the end row!!!!
    return read_row<T>( matname, startrow );
  }

  
  template <typename T>
  void hdf5_collection::add_to_matrix( const std::string& matname, const std::vector<std::string>& colnames, const std::vector< std::vector< T > >& vals )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }

    matrices[ locs[0] ].add_data<T>( colnames, vals );
  }

  template <typename T>
  void hdf5_collection::add_to_matrix( const std::string& matname, const std::vector< std::vector< T > >& vals )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }

    matrices[ locs[0] ].add_data<T>( vals );
  }

  
  template <typename T>
  void hdf5_collection::write_row_range( const std::string& matname, const size_t& startrow, const size_t& endrow, const std::vector< std::vector< T > >& vals )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    
    matrices[ locs[0] ].write_row_range<T>( startrow, endrow, vals );
    return;
  }

  template <typename T>
  void hdf5_collection::write_row( const std::string& matname, const size_t& startrow, const  std::vector< T > & vals )
  {
    
    std::vector<std::vector<T>> vals2( 1, vals );
    
    write_row_range( matname, startrow, startrow, vals2 );
    return;
  }
  
  template <typename T>
  std::vector<std::vector<T>> hdf5_collection::read_row_range( const std::string& matname, const size_t& startrow, const size_t& endrow )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    
    return matrices[ locs[0] ].read_row_range<T>( startrow, endrow );
  }

  template <typename T>
  std::vector<T> hdf5_collection::read_row( const std::string& matname, const size_t& row )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    
    return (matrices[ locs[0] ].read_row_range<T>( row, row )[0]);
  }

  void hdf5_collection::load_params()
  {
    //Just save it as char matrix OK.
    parameters = enumerate_hdf5_dir( file, "/" + PARAM_GRP_NAME );;
    fprintf(stdout, "Loaded parameters!\n");
    //print1dvec_row<std::string>( parameters );
    for(size_t x=0; x<parameters.size(); ++x)
      {
	fprintf(stdout, "[%s]\n", parameters[x].c_str());
      }
    return ;
  }
  
  //Opens existing as user specifies. Loads all the matrix_props etc. as user expects.
  void hdf5_collection::load_collection( const std::string& fname )
  {
    file = H5::H5File( fname, H5F_ACC_RDWR );
    file_name = fname;
    //Read all the datasets in there. That requires me enumerating them.
    
    load_params();
    
    std::vector<std::string> toload = matrix_names_from_file();
    fprintf(stdout, "LOAD COLLECTION: Got list of names of items to load.\n");
    for(size_t x=0; x<toload.size(); ++x)
      {
	fprintf( stdout, "[%s]\n", toload[x].c_str() );
      }
    
    for(size_t x=0; x<toload.size(); ++x)
      {
	if(toload[x].c_str()[0] != '/')
	  {
	    fprintf(stdout, "ERROR?!?!?! in load matrix [%s], first char is not slash?\n", toload[x].c_str() );
	    exit(1);
	  }
	fprintf(stdout, "Trying to load: [%s]\n", toload[x].c_str() );
	if( toload[x].size() > 1 )
	  {
	    //First char is '/'?!
	    if( !( toload[x].c_str()[1] == '_' && toload[x].c_str()[2] == '_') )
	      {
		fprintf(stdout, "Loading it because it is not a varnames i.e. __\n");
		load_matrix( toload[x] );
	      }
	    else
	      {
		fprintf(stdout, "SKIPPING because first 2 characters after / are underscores [%s]\n", toload[x].c_str());
	      }
	  }
	else if( toload[x].size() > 0 )
	  {
	    fprintf(stdout, "Loading it (it is of length 1)\n");
	    load_matrix( toload[x] );
	  }
	else
	  {
	    fprintf(stderr, "File name is empty!? [%s]\n", toload[x].c_str() );
	    exit(1);
	  }
      }
  }
  
  std::vector<std::string> hdf5_collection::matrix_names_from_file()
  {
    std::vector<std::string> datnames = enumerate_hdf5_dir( file, "/" );
    /*for(size_t x=0; x<datnames.size(); ++x)
      {
	fprintf(stdout, "DATASET [%ld]: [%s]\n", x, datnames[x].c_str() );
	}*/
    return datnames;
  }

  template <typename T>
  void hdf5_collection::enumerate_matrix( const std::string& matname )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    matrices[ locs[0] ].enumerate<T>();
  }

void hdf5_collection::enumerate_matrix_to_file( const std::string& matname, const std::string& fname,  const size_t& thinrate=1, const size_t& startpoint=0 )
  {
    std::vector< size_t > locs = find_matrix( matname );
    if(locs.size() != 1)
      {
	fprintf(stderr, "ERROR Couldn't find requested matrix name (dataset) [%s] in matrices, or there were multiple (Found [%ld])\n", matname.c_str(), locs.size());
      }
    
    FILE* fout = fopen2( fname.c_str(), "w" );
    
    //matrices[ locs[0] ].enumerate_to_file<T>( fname, thinrate );
    matrices[ locs[0] ].enumerate_to_file( fout, thinrate, startpoint );
    fclose2( fout );
  }
