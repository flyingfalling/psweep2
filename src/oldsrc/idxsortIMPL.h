#pragma once

#include "idxsort.h"

//REV: Rofl this is template implementation...
template <typename T>
sorter<T>::sorter(T* _targarray, int _size)
{
  targarray = _targarray;
  size = _size;
  
  indices = new int[_size];
  
  for(int i=0; i<size; ++i) /* REV: meh generators too complex */
    {
      indices[i] = i;
    }
}



template <typename T>
void sorter<T>::runsort()
{
  std::sort(indices, indices+size, sortthing<T>(targarray));
  
  /* REV: and now copy back */
  T* tmp = new T[size];
  std::memcpy(tmp, targarray, size*sizeof(T));
    
  std::vector<int*> tmpiargs;
  std::vector<long int*> tmplargs;
  std::vector<float*> tmpfargs;
  std::vector<double*> tmpdargs;
    
  for(int i=0; i<iargs.size(); ++i)
    {
      int* a = new int[size];
      std::memcpy(a, iargs[i], sizeof(int)*size);
      tmpiargs.push_back(a);
    }
  for(int i=0; i<largs.size(); ++i)
    {
      long int* a = new long int[size];
      std::memcpy(a, largs[i], sizeof(long int)*size);
      tmplargs.push_back(a);
    }
  for(int i=0; i<fargs.size(); ++i)
    {
      float* a = new float[size];
      std::memcpy(a, fargs[i], sizeof(float)*size);
      tmpfargs.push_back(a);
    }
  for(int i=0; i<dargs.size(); ++i)
    {
      double* a = new double[size];
      std::memcpy(a, dargs[i], sizeof(double)*size);
      tmpdargs.push_back(a);
    }
    
    
  /* REV: we are now all copied, now rearrange them into originals */
  for(int i=0; i<size; ++i)
    {
      targarray[i] = tmp[indices[i]];
    }
  for(int z=0; z<iargs.size(); ++z)
    {
      for(int i=0; i<size; ++i)
	{
	  iargs[z][i] = tmpiargs[z][indices[i]];
	}
      delete [] tmpiargs[z];
    }
  for(int z=0; z<largs.size(); ++z)
    {
      for(int i=0; i<size; ++i)
	{
	  largs[z][i] = tmplargs[z][indices[i]];
	}
      delete [] tmplargs[z];
    }
  for(int z=0; z<fargs.size(); ++z)
    {
      for(int i=0; i<size; ++i)
	{
	  fargs[z][i] = tmpfargs[z][indices[i]];
	}
      delete [] tmpfargs[z];
    }
  for(int z=0; z<dargs.size(); ++z)
    {
      for(int i=0; i<size; ++i)
	{
	  dargs[z][i] = tmpdargs[z][indices[i]];
	}
      delete [] tmpdargs[z];
    }
  
  delete [] indices;
  delete [] tmp;
} //end runsort()



