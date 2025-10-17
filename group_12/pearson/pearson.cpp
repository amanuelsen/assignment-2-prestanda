/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "analysis.hpp"
#include "dataset.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char const* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " [thread_count] [dataset] [outfile]" << std::endl;
        std::exit(1);
    }

    auto datasets { Dataset::read(argv[2]) };
    int thread_count = std::stoi(argv[1]);
    
    auto corrs { Analysis::correlation_coefficients(datasets) };
    Dataset::write(corrs, argv[3]);

    return 0;
}
