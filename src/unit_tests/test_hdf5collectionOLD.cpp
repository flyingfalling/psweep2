#include <hdf5_collection.h>



int main()
{
  std::string testfname = "test.h5";
  std::string dname = "dset1";

  hdf5_collection collection;
  collection.new_collection(testfname);

  matrix_props mp1;
  std::vector<std::string> varnames = { "Var1", "Var2", "Var3" };
  std::vector<std::vector<double> > dat1= {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  std::vector<std::vector<double> > dat2= {{7.0, 8.0, 9.0}, {10.0, 11.0, 12.0}, {13.0, 14.0, 15.0}};
  std::vector<std::vector<double> > dat3= {{16.0, 17.0, 18.0}};
  
  mp1.new_matrix(dname, varnames, collection.file);

  fprintf(stdout, "Adding data 1\n");
  mp1.add_data(varnames, dat1);
  mp1.enumerate();

  fprintf(stdout, "Adding data 2\n");
  mp1.add_data(varnames, dat2);
  mp1.enumerate();

  fprintf(stdout, "Adding data 3\n");
  mp1.add_data(varnames, dat3);
  mp1.enumerate();

  std::vector<std::vector<double> > ret = mp1.read_row_range( 2, 3 );

  fprintf(stdout, "Reading row range from 2 to 3 (i.e. 3rd to 4th row)\n");
  for(size_t x=0; x<ret.size(); ++x)
    {
      for(size_t y=0; y<ret[x].size(); ++y)
	{
	  fprintf(stdout, "%5.3f ", ret[x][y]);
	}
      fprintf(stdout, "\n");
    }


  ret = mp1.read_row_range( 5, 5 );

  fprintf(stdout, "Reading row range from 5 to 5 (i.e. only 6th row)\n");
  for(size_t x=0; x<ret.size(); ++x)
    {
      for(size_t y=0; y<ret[x].size(); ++y)
	{
	  fprintf(stdout, "%5.3f ", ret[x][y]);
	}
      fprintf(stdout, "\n");
    }

  ret = mp1.read_row_range( 7, 7 );

  fprintf(stdout, "Reading row range from 7 to 7 (i.e. only 8th row) (This should error and exit due to out of bounds)\n");
  for(size_t x=0; x<ret.size(); ++x)
    {
      for(size_t y=0; y<ret[x].size(); ++y)
	{
	  fprintf(stdout, "%5.3f ", ret[x][y]);
	}
      fprintf(stdout, "\n");
    }
}
