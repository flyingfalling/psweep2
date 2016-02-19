


//Open a dataset. Write 1 row to it. Read every row one at a time. Do this a gazillion times.

//Because what the heck? Hopefully it won't cache it.


void writerow( H5::DataSet& ds, const std::vector<double>& rowtowrite )
{
  
}

void readallrows( H5::DataSet& ds )
{
  
}

int main()
{
  std::vector<double> rowtowrite( 100, 5.0 );

  H5::H5File f;

  H5::DataSet ds;
  
  for(size_t t=0; t<10000000; ++t)
    {
      writerow( ds, rowtowrite );
      readallrows( ds );
    }
}
