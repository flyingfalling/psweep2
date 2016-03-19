#pragma once

#include <commontypes.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>


std::vector<size_t> findlegaldevs_byname(const std::string& devname);

void set_cuda_device( const size_t& idx );
