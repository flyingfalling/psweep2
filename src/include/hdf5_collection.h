
#pragma once

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <H5Cpp.h>



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
    //std::string name; //we use this as name of thing in H5 file.
    //size_t nrows;
    //size_t ncols;
    //std::vector<std::string> colnames;

    //ACTUALLY allocated (with fill)
    //USED length (i.e. filled with my data)
    
    H5::DataSet dataset;
    //DataSpace dataspace;
    
    void new_matrix( const std::string& dsetname, const std::vector<std::string>& _colnames, H5::H5File& f )
    {
            
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
      
    }


    //modify dataset also

    void overwrite_data()
    {
      
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

      fprintf(stdout, "Got original data space (before extend). [%d] dims: [%lld] row [%lld] col\n", ndims, dims_out[0], dims_out[1] );
      
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

      fprintf(stdout, "Finished writing\n");
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
      fprintf(stdout, "Will enum data set:\n\n");
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
    

    void load_matrix()
    {
      
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

  

  //Opens existing as user specifies. Loads all the matrix_props etc. as user expects.
  void load_collection( const std::string& fname )
  {
    
  }
  
};
