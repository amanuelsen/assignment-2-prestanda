/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "vector.hpp"
#include <vector>

#if !defined(ANALYSIS_PAR_HPP)
#define ANALYSIS_PAR_HPP

namespace Analysis_par {
std::vector<double> correlation_coefficients(std::vector<Vector>& datasets, int thread_count);
double pearson(const Vector& vec1, const Vector& vec2);
};

#endif
