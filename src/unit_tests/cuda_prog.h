#pragma once

#include <cstdlib>
#include <vector>
#include <commontypes.h>

std::vector<size_t> find_legaldevs();
std::vector<float64_t> gpucomp( std::vector<float64_t>& est, std::vector<float64_t>& actual, size_t& cudadevnum );
