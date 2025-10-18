/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "analysis.hpp"
#include "dataset.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char const* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [dataset] [outfile] [thread_count]" << std::endl;
        std::exit(1);
    }

    auto datasets { Dataset::read(argv[1]) };

    int thread_count = std::stoi(argv[3]);
    auto corrs { Analysis::correlation_coefficients(datasets, thread_count) };
    Dataset::write(corrs, argv[2]);

    return 0;
}
