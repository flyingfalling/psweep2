

#include <hdf5_collection.h>


void dotest( hdf5_collection& col )
{

  std::string mat1n = "mat1";
  std::string mat2n = "mat2";
  
  std::vector<std::vector<double> > ret = col.read_row_range<double>( mat1n, 2, 3 );

  fprintf(stdout, "Reading row range from 2 to 3 of matrix 1 (i.e. 3rd to 4th row)\n");
  for(size_t x=0; x<ret.size(); ++x)
    {
      for(size_t y=0; y<ret[x].size(); ++y)
	{
	  fprintf(stdout, "%5.3lf ", ret[x][y]);
	}
      fprintf(stdout, "\n");
    }


  std::vector<double> ret2 = col.read_row<double>( mat2n, 3 );

  fprintf(stdout, "Reading row range 3 from matrix 2 (i.e. only 4th row)\n");
  for(size_t x=0; x<ret.size(); ++x)
    {
      fprintf(stdout, "%5.3lf ", ret2[x]);
    }
  fprintf(stdout, "\n");


  fprintf(stdout, "Got STRPARAM (should be STRPARAM_VAL): [%s]\n", col.get_string_parameter("STRPARAM").c_str() );//, "STRPARAM_VAL");
  fprintf(stdout, "Got FPARAM (should be 666.6): [%lf]\n", col.get_float64_parameter("F64PARAM") ); //, 666.6);
  fprintf(stdout, "Got IPARAM (should be 2): [%ld]\n", col.get_int64_parameter("I64PARAM") ); //, 2);
  
}


void loadfile(const std::string& testfname)
{
  hdf5_collection col;
  col.load_collection(testfname);
  fprintf(stdout, "\n\n\n\n\n\n\n LOADED NOW!\n");
  dotest( col );
}

void buildfile(const std::string& testfname)
{
  
  hdf5_collection col;
  col.new_collection(testfname);
  
  std::vector<std::string> varnames1 = { "Var1", "Var2", "Var3" };
  std::vector<std::string> varnames2 = { "Var1", "Var2" };
  std::vector<std::vector<double> > dat1_1= {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  std::vector<std::vector<double> > dat1_2= {{7.0, 8.0, 9.0}, {10.0, 11.0, 12.0}, {13.0, 14.0, 15.0}};
  std::vector<std::vector<double> > dat1_3= {{16.0, 17.0, 18.0}};

  std::vector<std::vector<double> > dat2_1= {{1.0, 2.0}, {4.0, 5.0}};
  std::vector<std::vector<double> > dat2_2= {{6.0, 7.0}, {8.0, 9.0}};

  std::string mat1n = "mat1";
  std::string mat2n = "mat2";
  
  col.add_new_matrix( mat1n, varnames1, "REAL" );
  col.add_new_matrix( mat2n, varnames2, "REAL" );


  //add to mat1
  col.add_to_matrix<double>(mat1n, varnames1, dat1_1);
  col.enumerate_matrix<double>( mat1n );
  col.enumerate_matrix<double>( mat2n );

  //add to mat2
  col.add_to_matrix<double>(mat2n, varnames2, dat2_1);
  col.enumerate_matrix<double>( mat1n );
  col.enumerate_matrix<double>( mat2n );

  //add to mat1
  col.add_to_matrix<double>(mat1n, varnames1, dat1_2);
  col.enumerate_matrix<double>( mat1n );
  col.enumerate_matrix<double>( mat2n );

  //add to mat2
  col.add_to_matrix<double>(mat2n, varnames2, dat2_2);
  col.enumerate_matrix<double>( mat1n );
  col.enumerate_matrix<double>( mat2n );

  //add to mat1
  col.add_to_matrix<double>(mat1n, varnames1, dat1_3);
  col.enumerate_matrix<double>( mat1n );
  col.enumerate_matrix<double>( mat2n );

  fprintf(stdout, "REV: Making parameters.\n");
  
  col.add_string_parameter("STRPARAM", "STRPARAM_VAL");
  col.add_float64_parameter("F64PARAM", 666.6);
  col.add_int64_parameter("I64PARAM", 2);

  col.backup();
  
  dotest( col );


}

int main()
{
  std::string testfname = "test.h5";

  buildfile(testfname);
  //At end it should destruct it right?

  loadfile(testfname);

  
  //auto a = col.matrix_names_from_file();
  
  //Close it and reload the file? First run it.
}
