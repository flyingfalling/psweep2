#pragma once
#include <vector>
#include <cstdio>

template <typename T>
T vector_max(const std::vector<T>& arg)
{
  if(arg.size() > 0)
    {
      T max = arg[0];
      for(size_t x=0; x<arg.size(); ++x)
	{
	  if( arg[x] > max )
	    {
	      max = arg[x];
	    }
	}
      return max;
    }
  else
    {
      fprintf(stderr, "ERROR: vector_max: attempting to find max of 0 length vector\n");
      exit(1);
    }
}


template <typename T>
T vector_sum(const std::vector<T>& arg)
{
  T sum = 0;
  
  for(size_t x=0; x<arg.size(); ++x)
    {
      sum += arg[x];
    }
  
  return sum;
}




template <typename T>
void vector_add_constant(std::vector<T>& arg, const T& constant)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      arg[x] += constant;
    }
}

template <typename T>
void vector_subtract_constant(std::vector<T>& arg, const T& constant)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      arg[x] -= constant;
    }
}
  

template <typename T>
std::vector<T> vector_subtract(const std::vector<T>& arg1, const std::vector<T>& arg2)
{
  if(arg1.size() != arg2.size())
    {
      fprintf(stderr, "ERROR IN VECTOR_SUBTRACT: arg1 and arg2 vectors are different lengths...(%ld vs %ld)\n", arg1.size(), arg2.size()); exit(1);
    }
  std::vector<T> ret(arg1.size());
  for(size_t x=0; x<arg1.size(); ++x)
    {
      ret[x] = arg1[x] - arg2[x];
    }
  return ret;
}

template <typename T>
std::vector<T> vector_add(const std::vector<T>& arg1, const std::vector<T>& arg2)
{
  if(arg1.size() != arg2.size())
    {
      fprintf(stderr, "ERROR IN VECTOR_ADD: arg1 and arg2 vectors are different lengths...(%ld vs %ld)\n", arg1.size(), arg2.size()); exit(1);
    }
  std::vector<T> ret(arg1.size());
  for(size_t x=0; x<arg1.size(); ++x)
    {
      ret[x] = arg1[x] + arg2[x];
    }
  return ret;
}
  

template <typename T>
std::vector<T> vector_multiply(const std::vector<T>& arg1, const std::vector<T>& arg2)
{
  if(arg1.size() != arg2.size())
    {
      fprintf(stderr, "ERROR IN VECTOR_MULTIPLY: arg1 and arg2 vectors are different lengths...(%ld vs %ld)\n", arg1.size(), arg2.size()); exit(1);
    }
  std::vector<T> ret(arg1.size());
  for(size_t x=0; x<arg1.size(); ++x)
    {
      ret[x] = arg1[x] * arg2[x];
    }
  return ret;
}


template <typename T>
std::vector<T> vector_divide(const std::vector<T>& arg1, const std::vector<T>& arg2)
{
  if(arg1.size() != arg2.size())
    {
      fprintf(stderr, "ERROR IN VECTOR_DIVIDE: arg1 and arg2 vectors are different lengths...(%ld vs %ld)\n", arg1.size(), arg2.size()); exit(1);
    }
  std::vector<T> ret(arg1.size());
  for(size_t x=0; x<arg1.size(); ++x)
    {
      ret[x] = arg1[x] / arg2[x];
    }
  return ret;
}
  
  

template <typename T>
void vector_divide_constant(std::vector<T>& arg, const T& constant)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      arg[x] /= constant;
    }
}

template <typename T>
void vector_multiply_constant(std::vector<T>& arg, const T& constant)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      arg[x] *= constant;
    }
}


template <typename T>
void vector_exponent(std::vector<T>& arg)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      arg[x] = exp(arg[x]);
    }
}

template <typename T>
void vector_sqrt(std::vector<T>& arg)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      arg[x] = sqrt(arg[x]);
    }
}

template <typename T>
void vector_sq(std::vector<T>& arg)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      arg[x] = arg[x] * arg[x];
    }
}


template <typename T>
void print2dvec_row(const std::vector< std::vector<T> >& arg)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      for(size_t z=0; z<arg[x].size(); ++z)
	{
	  //REV: This will only work for floats haha...
	  std::fprintf(stdout, "%05.3lf ", arg[x][z]);
	}
      std::fprintf(stdout, "\n");
    }
}


template <typename T>
void print1dvec_row(const std::vector<T>& arg)
{
  for(size_t x=0; x<arg.size(); ++x)
    {
      //std::fprintf(stdout, "%05.3f ", arg[x]);
      std::cout << arg[x] << " ";
    }
  std::fprintf(stdout, "\n");
}
