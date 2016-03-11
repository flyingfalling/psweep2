#pragma once

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <random> //for random testing
#include <cstring> //for memcpy
#include <algorithm> // For sort


//void testidxsort();




template <typename T>
class sortthing
{
 private:
  T* dat;
 
 public:
  bool operator()  (int i, int j) { return (dat[i] < dat[j]); }
  sortthing(T* da) : dat(da) {}
};



template <typename T>
struct sorter
{
  sorter(T* targarray, int size, std::vector<int*> iargs, std::vector<int*> largs, std::vector<float*> fargs, std::vector<double*> dargs);
  sorter(T* targarray, int size); 
  
  void runsort();

  void add_l(long int* a);
  
  void add_i(int* a);
 
  
  void add_f(float* a);
 

  void add_d(double* a);
  
  
  
  int* indices;
  T* targarray;
  std::vector<int*> iargs;
  std::vector<long int*> largs;
  std::vector<float*> fargs;
  std::vector<double*> dargs;
  int size;
  
  
};

