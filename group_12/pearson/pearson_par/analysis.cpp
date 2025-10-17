/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "analysis.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

// used for multithreading
#include <pthread.h>

namespace Analysis {

// struct to hold job and result for a thread
struct ThreadData {
    const std::vector<Vector>* datasets;
    std::vector<std::pair<int, int>> pairs;
    std::vector<double> results;
};

// thread function
void* thread_worker(void* arg)
{
    ThreadData* data = static_cast<ThreadData*>(arg);

    for (auto [i, j] : data->pairs) {
        data->results.push_back(pearson((*data->datasets)[i], (*data->datasets)[j]));
    }

    return nullptr;
}

// added referencing to each vector input, avoiding copyconstructor usage
std::vector<double> correlation_coefficients(std::vector<Vector>& datasets, int thread_count)
{
    std::vector<std::pair<int, int>> all_pairs;

    // Generate all unique (i, j) pairs where i < j
    for (int i = 0; i < datasets.size() - 1; ++i) {
        for (int j = i + 1; j < datasets.size(); ++j) {
            all_pairs.emplace_back(i, j);
        }
    }

    // Split pairs among threads
    std::vector<ThreadData> thread_data(thread_count);
    for (int t = 0; t < thread_count; ++t) {
        thread_data[t].datasets = &datasets;
    }

    for (size_t i = 0; i < all_pairs.size(); ++i) {
        thread_data[i % thread_count].pairs.push_back(all_pairs[i]);
    }

    std::vector<pthread_t> threads(thread_count);

    // Launch threads
    for (int t = 0; t < thread_count; ++t) {
        pthread_create(&threads[t], nullptr, thread_worker, &thread_data[t]);
    }

    // Join threads
    for (int t = 0; t < thread_count; ++t) {
        pthread_join(threads[t], nullptr);
    }

    // Collect results
    std::vector<double> result;
    for (const auto& td : thread_data) {
        result.insert(result.end(), td.results.begin(), td.results.end());
    }

    return result;
}

// added referencing to each vector input, avoiding copyconstructor usage
double pearson(const Vector& vec1, const Vector& vec2)
{
    auto x_mean { vec1.mean() };
    auto y_mean { vec2.mean() };

    auto x_mm { vec1 - x_mean };
    auto y_mm { vec2 - y_mean };

    auto x_mag { x_mm.magnitude() };
    auto y_mag { y_mm.magnitude() };

    auto x_mm_over_x_mag { x_mm / x_mag };
    auto y_mm_over_y_mag { y_mm / y_mag };

    auto r { x_mm_over_x_mag.dot(y_mm_over_y_mag) };

    return std::max(std::min(r, 1.0), -1.0);
}
};
