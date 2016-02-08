#include <hdf5_collection.h>



int main()
{
  std::string testfname = "test.h5";
  std::string dname = "dset1";

  hdf5_collection collection;
  collection.new_collection(testfname);

  collection::matrix_props mp1;
  std::vector<std::string> varnames = { "Var1", "Var2", "Var3" };
  std::vector<std::vector<double> > dat1= {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  std::vector<std::vector<double> > dat2= {{7.0, 8.0, 9.0}, {10.0, 11.0, 12.0}, {13.0, 14.0, 15.0}};
  std::vector<std::vector<double> > dat3= {{16.0, 17.0, 18.0}};
  
  mp1.new_matrix(dname, varnames);

  fprintf(stdout, "Adding data 1\n");
  mp1.add_data(varnames, dat1);
  mp1.enumerate();

  fprintf(stdout, "Adding data 2\n");
  mp1.add_data(varnames, dat2);
  mp1.enumerate();

  fprintf(stdout, "Adding data 3\n";)
  mp1.add_data(varnames, dat3);
  mp1.enumerate();
}
