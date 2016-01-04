#pragma once

#include "idxsort.h"


/*int main()
{
  testidxsort();
  }*/

//template <typename T>
/*void testidxsort()
{
  
  std::default_random_engine gen;
  std::uniform_int_distribution<int> dist(1,99);
  
  int size=300;
  int tosort[size];
  float tosortf[size];
  int reorg[size];
  
  for(int i=0; i<size; ++i)
    {
      tosort[i] = dist(gen);
      tosortf[i] = (float)dist(gen);
      reorg[i] = dist(gen);
      
      fprintf(stdout, "%2d  ||  %5.1f  |  %2d\n", tosort[i], tosortf[i], reorg[i]);
    }
  fprintf(stdout, "\n");
  
  
  //std::vector<int*> i; 
  //std::vector<float*> f;
  //std::vector<double*> d;
  
  //  i.push_back(reorg);
  //f.push_back(tosortf);
  sorter<int> a(tosort, size); //, i, f, d);
  
  a.add_i(reorg);
  a.add_f(tosortf);
  
  
  a.runsort();
  
  fprintf(stdout, "SORTED\n\n");
  for(int i=0; i<size; ++i)
    {
      
      fprintf(stdout, "%2d  ||  %5.1f  |  %2d\n", tosort[i], tosortf[i], reorg[i]);
    }
  fprintf(stdout, "\n");
  
  
}
*/


template <typename T>
sorter<T>::sorter(T* _targarray, int _size, std::vector<int*> _iargs, std::vector<float*> _fargs, std::vector<double*> _dargs)
{
  targarray = _targarray;
  size = _size;
  fargs = _fargs;
  dargs = _dargs;
  iargs = _iargs;
  
  //indices.resize(size);
  
  indices = new int[_size];
  
  for(int i=0; i<size; ++i) /* REV: meh generators too complex */
    {
      indices[i] = i;
    }
}


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
  std::vector<float*> tmpfargs;
  std::vector<double*> tmpdargs;
    
  for(int i=0; i<iargs.size(); ++i)
    {
      int* a = new int[size];
      std::memcpy(a, iargs[i], sizeof(int)*size);
      tmpiargs.push_back(a);
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



