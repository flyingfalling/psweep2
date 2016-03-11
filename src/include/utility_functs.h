//REV: some utility functs (wrappers) to deal with some common problems especially with lustre etc.

#pragma once

#ifndef UTILITY_FUNCTS_H
#define UTILITY_FUNCTS_H


//for errno
#include <errno.h>
#include <cstdlib>
#include <cstdio>

//for std::string
#include <string>
# include <iostream>
# include <fstream>
# include <iomanip>
#include <unistd.h> 
#include <sys/types.h>
#include <cstring>

#include <cerrno>

#include <sys/stat.h>

#include <vector>


//check file existence?
#include <fcntl.h> //for the open stuff. Another option is to use stat
#include <errno.h>
#include <unistd.h> 

#include <sys/sendfile.h>  // sendfile
#include <fcntl.h>         // open
#include <unistd.h>        // close
#include <sys/stat.h>      // fstat
#include <sys/types.h>     // fstat
#include <ctime>

void copy_file(const std::string& src, const std::string& targ);


void copy_fileBAD( const std::string& src, const std::string& targ );



std::vector<size_t> find_string_in_vect( const std::string& targ, const std::vector<std::string>& vect );



bool check_file_existence( const std::string& fname );


//Gets current user's mask?
mode_t getumask();


bool make_directory( const std::string& s );


//REV: ghetto function to read a whole file into a single std::string
std::string get_file_contents(const std::string filename);


//REV: modified this to remove "mode" since it will always just be READ?
void open_ifstream( std::string fname, std::ifstream& f, std::ios_base::openmode mode=std::ios_base::in, size_t tries=10, long timeout_us=10000 );


//inline void open_ofstream( std::string fname, std::ofstream& f, std::ios_base::openmode mode=std::ios_base::app, size_t tries=10, long timeout_us=10000 )
void open_ofstream( std::string fname, std::ofstream& f, std::ios_base::openmode mode=std::ios_base::app, size_t tries=10, long timeout_us=10000 );

//void open_fstream( std::fstream& f, ios_base::openmode mode )
void open_fstream( std::string fname, std::fstream& f, std::ios_base::openmode mode =  std::ios_base::app | std::ios_base::in, size_t tries=10, long timeout_us=10000 );

void close_ifstream( std::ifstream& f, size_t tries=10, long timeout_us=10000 );

void close_ofstream( std::ofstream& f, size_t tries=10, long timeout_us=10000 );

void close_fstream( std::fstream& f, size_t tries=10, long timeout_us=10000 );

FILE* fopen2(const char* fn, std::string m, size_t tries=10, long timeout_us=10000 );

void fclose2( FILE* f, size_t tries=10, long timeout_us=10000 );

#endif //UTILITY_FUNCTS_H
