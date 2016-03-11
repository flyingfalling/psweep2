#pragma once
#include <vector>
#include <cstdio>
#include <cmath>

template <typename T>
T vector_sum(const std::vector<T>& arg);




template <typename T>
void vector_add_constant(std::vector<T>& arg, const T& constant);


template <typename T>
void vector_subtract_constant(std::vector<T>& arg, const T& constant);


template <typename T>
void vector_subtract_from_constant( const T& constant, std::vector<T>& arg);

  

template <typename T>
std::vector<T> vector_subtract(const std::vector<T>& arg1, const std::vector<T>& arg2);


template <typename T>
std::vector<T> vector_add(const std::vector<T>& arg1, const std::vector<T>& arg2);


template <typename T>
std::vector<T> vector_multiply(const std::vector<T>& arg1, const std::vector<T>& arg2);



template <typename T>
std::vector<T> vector_divide(const std::vector<T>& arg1, const std::vector<T>& arg2);

  

template <typename T>
void vector_divide_constant(std::vector<T>& arg, const T& constant);

template <typename T>
void vector_multiply_constant(std::vector<T>& arg, const T& constant);


template <typename T>
void vector_exponent(std::vector<T>& arg);

template <typename T>
void vector_sqrt(std::vector<T>& arg);

template <typename T>
void vector_sq(std::vector<T>& arg);

template <typename T>
size_t vector_min_pos( const std::vector<T>& arg );

template <typename T>
size_t vector_max_pos( const std::vector<T>& arg );


template <typename T>
T vector_min( const std::vector<T>& arg );


template <typename T>
T vector_max( const std::vector<T>& arg );




template <typename T>
void print2dvec_row(const std::vector< std::vector<T> >& arg);


template <typename T>
void print1dvec_row(const std::vector<T>& arg);


void print1d_str_vec_row(const std::vector<std::string>& arg);


#include <vector_utils.cpp>
