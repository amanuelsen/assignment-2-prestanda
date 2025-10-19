/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "analysis_par.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

// used for multithreading
#include <pthread.h>

namespace Analysis_par {

// struct to hold job and result for a thread
struct ThreadData {
    const std::vector<Vector>* datasets;
    std::vector<std::pair<int, int>> pairs;
    std::vector<double>* results;
    size_t offset;
};

// thread function
void* thread_worker(void* arg)
{
    ThreadData* data = static_cast<ThreadData*>(arg);

    // loop over pairs and calculate pearson, result back to shared result vector with offset
    for (size_t k = 0; k < data->pairs.size(); ++k) {
        auto [i, j] = data->pairs[k];
        (*data->results)[data->offset + k] = pearson((*data->datasets)[i], (*data->datasets)[j]);
    }

    return nullptr;
}

// compute all correlation coefficients using multithreading
std::vector<double> correlation_coefficients(std::vector<Vector>& datasets, int thread_count)
{
    std::vector<std::pair<int, int>> all_pairs;

    // generate all unique (i, j) same as before
    for (int i = 0; i < static_cast<int>(datasets.size()) - 1; ++i) {
        for (int j = i + 1; j < static_cast<int>(datasets.size()); ++j) {
            all_pairs.emplace_back(i, j);
        }
    }

    size_t total_pairs = all_pairs.size();

    // how many pairs each thread should handle
    size_t chunk_size = (total_pairs + thread_count - 1) / thread_count;

    std::vector<ThreadData> thread_data(thread_count);
    std::vector<pthread_t> threads(thread_count);
    std::vector<double> result(total_pairs);

    // split pairs among all threads and setup data
    for (int t = 0; t < thread_count; ++t) {
        // calculate start & end for the threads work
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, total_pairs);

        thread_data[t].datasets = &datasets;
        thread_data[t].results = &result;
        thread_data[t].offset = start;
        thread_data[t].pairs.assign(all_pairs.begin() + start, all_pairs.begin() + end);
    }

    // create threads
    for (int t = 0; t < thread_count; ++t) {
        pthread_create(&threads[t], nullptr, thread_worker, &thread_data[t]);
    }

    // join threads
    for (int t = 0; t < thread_count; ++t) {
        pthread_join(threads[t], nullptr);
    }

    return result;
}

// calculate Pearson correlation between two vectors, same as before
double pearson(const Vector& vec1, const Vector& vec2)
{
    auto x_mean = vec1.mean();
    auto y_mean = vec2.mean();

    auto x_mm = vec1 - x_mean;
    auto y_mm = vec2 - y_mean;

    auto x_mag = x_mm.magnitude();
    auto y_mag = y_mm.magnitude();

    auto x_mm_over_x_mag = x_mm / x_mag;
    auto y_mm_over_y_mag = y_mm / y_mag;

    auto r = x_mm_over_x_mag.dot(y_mm_over_y_mag);

    return std::max(std::min(r, 1.0), -1.0);
}

};
