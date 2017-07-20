
#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <stack>

#include <boost/tokenizer.hpp>

std::string CONCATENATE_STR_ARRAY(const std::vector<std::string>& arr, const std::string& sep);

std::vector<std::string> tokenize_string(const std::string& src, const std::string& delim, bool include_empty_repeats=false);



/* keep empty, i.e. do I want to know when it is e.g. :: or //? */
//std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats=false);

std::string get_canonical_dir_of_fname( const std::string& s, std::string& fnametail );

std::string canonicalize_fname( const std::string& s );


bool same_fnames(const std::string& fname1, const std::string& fname2);


void replace_old_fnames_with_new( std::vector<std::string>& fvect, const std::string& newfname, const std::vector<size_t> replace_locs );

//This only matches EXACT filenames
std::vector<size_t> find_matching_files(const std::string& fname, const std::vector<std::string>& fnamevect, std::vector<bool>& marked );
