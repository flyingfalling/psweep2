#include <H5Cpp.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <cstdint>

//Open a dataset. Write 1 row to it. Read every row one at a time. Do this a gazillion times.

//Because what the heck? Hopefully it won't cache it.


void addrow( H5::DataSet& ds, const std::vector<double>& rowtowrite )
{
  //Get the space (since it may have grown in length since last time of course )
  H5::DataSpace origspace = ds.getSpace();

  //get the rank, even though I know it is 2
  int rank = origspace.getSimpleExtentNdims();

  //Get the actual dimensions of the ranks.
  hsize_t dims[rank];
  int ndims = origspace.getSimpleExtentDims( dims, NULL);

  //Want to ADD a row, so need to offset at row = nrows, and col = 0;
  hsize_t offset[rank] = { dims[0], 0 }; 
  hsize_t dims_toadd[rank] = { 1, rowtowrite.size() }; //will write 1 row, ncols columns.

  //Compute "new" size (extended by 1 row).
  hsize_t size[rank] = { dims[0]+dims_toadd[0], rowtowrite.size() };

  //Do the extension.
  ds.extend( size );
  
  //Get the new (extended) space, and select the hyperslab to write the row to.
  origspace = ds.getSpace();
  origspace.selectHyperslab( H5S_SELECT_SET, dims_toadd, offset );

  //Make the "memory" data space?
  H5::DataSpace toaddspace(rank, dims_toadd);

  ds.write(  rowtowrite.data(), H5::PredType::NATIVE_DOUBLE, toaddspace, origspace );

  //Can close toaddspace/origspace with no effect.
  //Can also close/open data set at the beginning of each time with no effect.
}

hsize_t getnrows( H5::DataSet& ds )
{

  //REV: Hack to switch getSpace(); We want to manually drop the ID? Leak in 1.8.15...
  //We do this by getting ID of ds, then manually getting space of it, and creating DataSpace with that ID?
  hid_t id2 = ds.getId();
  hid_t myspace = H5Dget_space(id2);
  
  //H5::DataSpace origspace = ds.getSpace();
  H5::DataSpace origspace( myspace );
  
  int rank = origspace.getSimpleExtentNdims();
  hsize_t dims[rank];
  int ndims = origspace.getSimpleExtentDims( dims, NULL);
  hsize_t nrows=dims[0];
  hsize_t ncols=dims[1];

  return nrows;
}

std::vector<double> read1row( H5::DataSet& ds, const hsize_t& row )
{
  H5::DataSpace origspace = ds.getSpace();
  int rank = origspace.getSimpleExtentNdims();
  hsize_t dims[rank];
  int ndims = origspace.getSimpleExtentDims( dims, NULL);
  hsize_t nrows=dims[0];
  hsize_t ncols=dims[1];
  std::vector<double> returnvect( ncols );

  if(row >= nrows )
    {
      fprintf(stderr, "REV: ERROR, trying to read a row outside of matrix ([%lld] vs mat size [%lld])\n", row, nrows);
      exit(1);
    }
  
  hsize_t targrowoffset = row;
  hsize_t targcoloffset = 0;
  hsize_t dimsmem[rank] = {1,  ncols};
  H5::DataSpace memspace(rank, dimsmem);

  int id=memspace.getId();
  if(id==INT_MAX)
    {
      fprintf(stdout, "WOW, ID == INT_MAX, errortime?\n");
    }

  if( (row+1) % 100000 == 0 )
    {
      fprintf(stdout, "MEMSPACE ID [%d]\n", memspace.getId() );
    }
  
  hsize_t offset[rank] = { targrowoffset, targcoloffset };
  origspace.selectHyperslab( H5S_SELECT_SET, dimsmem, offset );
  ds.read( returnvect.data(), H5::PredType::NATIVE_DOUBLE, memspace, origspace );
  
  return returnvect;
}

void readallrows( H5::DataSet& ds )
{
  hsize_t nrows = getnrows( ds );
  //fprintf(stdout, "Read [%lld] rows\n", nrows );
  for(hsize_t r=0; r<nrows; ++r)
    {
      std::vector<double> gotvect = read1row( ds, r );
      if( gotvect[0] != (double)r )
	{
	  fprintf(stderr, "ERROR IN READING...\n");
	  exit(1);
	}
    }
  return;
}

std::vector<double> readlastrow( H5::DataSet& ds )
{
  H5::DataSpace origspace = ds.getSpace();
  int rank = origspace.getSimpleExtentNdims();
  hsize_t dims[rank];
  int ndims = origspace.getSimpleExtentDims( dims, NULL);
  hsize_t nrows=dims[0];
  hsize_t ncols=dims[1];
  std::vector<double> returnvect( ncols );

  
  
  hsize_t targrowoffset = nrows-1;
  hsize_t targcoloffset = 0;
  hsize_t dimsmem[rank] = {1,  ncols};
  H5::DataSpace memspace(rank, dimsmem);

  hsize_t offset[rank] = { targrowoffset, targcoloffset };
  origspace.selectHyperslab( H5S_SELECT_SET, dimsmem, offset );
  ds.read( returnvect.data(), H5::PredType::NATIVE_DOUBLE, memspace, origspace );

  return returnvect;
}

//H5::DataSpace memspace;
//H5::DataSpace origspace;

int fakereadlastrow( H5::DataSet& ds, const int& previd )
{
  
  hid_t id2 = ds.getId();
  hid_t myspace = H5Dget_space(id2);
  H5::DataSpace origspace( myspace );
  H5Sclose( myspace );
  //H5::DataSpace origspace = ds.getSpace();

  int rank = origspace.getSimpleExtentNdims();
  hsize_t dims[rank];
  int ndims = origspace.getSimpleExtentDims( dims, NULL);
  hsize_t nrows=dims[0];
  hsize_t ncols=dims[1];
  std::vector<double> returnvect( ncols );
  
  
  
  hsize_t targrowoffset = nrows-1;
  hsize_t targcoloffset = 0;
  hsize_t dimsmem[rank] = {1,  ncols};
  //H5::DataSpace memspace(rank, dimsmem);
  //H5::DataSpace* memspace = new H5::DataSpace(rank, dimsmem);
  
  H5::DataSpace memspace(rank, dimsmem);
    //memspace = ds.getSpace();
    //memspace.setExtentSimple(rank, dimsmem);
  
  hsize_t offset[rank] = { targrowoffset, targcoloffset };
  origspace.selectHyperslab( H5S_SELECT_SET, dimsmem, offset );
  
  //REV: Would read here, but I don't for speed.
  //ds.read( returnvect.data(), H5::PredType::NATIVE_DOUBLE, memspace, origspace );
  int id =   memspace.getId();
  //int id =   memspace->getId();

  if(id % 1000000 == 0 )
    {
     fprintf(stdout, "PREV ID: [%d] now ID: [%d] (origspace is: [%d])\n", previd, id, origspace.getId());
   }

  
  //origspace.close();
  //memspace.close();
  
  //memspace->close();
  //delete memspace;
  //memspace.close();
  //memspace.~DataSpace();

  //origspace.close();
  //memspace.close();
  
  return id;
  
  
  //return returnvect;
}

int main()
{
  std::string fname = "testhdf5file.h5";
  H5::H5File f( fname, H5F_ACC_TRUNC );
  std::string dsetname = "dset1";

  const hsize_t nranks = 2;
  const hsize_t ncols = 20;
  const hsize_t nrows = 0; //start with zero rows.
      
  hsize_t  dims[nranks] = {nrows, ncols};
  hsize_t  max_dims[nranks] = {H5S_UNLIMITED, ncols};
  
  H5::DataSpace dataspace( nranks, dims, max_dims );
  H5::DSetCreatPropList prop; //could set properties, but whatever.
  
  const hsize_t nrows_chunk = 1; //Need to mess with CACHE size too!
  hsize_t  chunk_dims[nranks] = { nrows_chunk, ncols};
  prop.setChunk(nranks, chunk_dims);


  //Create the dataset 
  H5::DataSet ds = f.createDataSet( dsetname, H5::PredType::NATIVE_DOUBLE,
				    dataspace, prop);
  

  size_t nrowstoadd=1;
  
  for(size_t t=0; t<nrowstoadd; ++t)
    {
      std::vector<double> rowtowrite( ncols, (double)t );
      addrow( ds, rowtowrite );
    }

  int id=0;
  for(size_t t=0; t<10000000; ++t)
    {
      
      //readallrows( ds );
      
      //readlastrow();
      
      id = fakereadlastrow(ds, id);
	
      
      //f.flush(H5F_SCOPE_GLOBAL);
    }
  fprintf(stdout, "Finished all iters!\n");
  
  return 0;
}
