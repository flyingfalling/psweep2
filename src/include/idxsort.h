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
  inline bool operator()  (int i, int j); 
  inline sortthing(T* da);
};



template <typename T>
struct sorter
{
  inline sorter(T* targarray, int size, std::vector<int*> iargs, std::vector<int*> largs, std::vector<float*> fargs, std::vector<double*> dargs);
  inline sorter(T* targarray, int size); 
  
 inline  void runsort();

 inline  void add_l(long int* a);
  
 inline  void add_i(int* a);
 
  
 inline  void add_f(float* a);
 

 inline  void add_d(double* a);
  
  
  
  int* indices;
  T* targarray;
  std::vector<int*> iargs;
  std::vector<long int*> largs;
  std::vector<float*> fargs;
  std::vector<double*> dargs;
  int size;
  
  
};


#include <idxsort.cpp>
