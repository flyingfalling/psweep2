

#include <H5Cpp.h>


int main()
{
  H5::H5File file = H5::H5File("test.h5", H5F_ACC_TRUNC);
  hsize_t numdims=1;
  std::string pname = "TESTATTRIBUTE";
  hsize_t DIM1length=1;
  hsize_t dims[numdims] = { DIM1length };

  H5::DataSpace attr_dataspace(1, dims );

  H5::DataSpace ds_space(1, dims ); //same, cuz just 1x1
  H5::DataSet dataset = file.createDataSet("TESTDSET", H5::PredType::NATIVE_DOUBLE, ds_space );

  
  H5::Attribute attribute = dataset.createAttribute( pname.c_str(),
						     H5::PredType::NATIVE_LONG, 
						     attr_dataspace);
  long value=100;
  attribute.write( H5::PredType::NATIVE_LONG, &value );

  //I can even try to write the dataset even though it shouldn't matter what it has in it?
  int i=1;
  dataset.write( &i, H5::PredType::NATIVE_INT );
  dataset.close();
  file.close();
}
