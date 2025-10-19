/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>

namespace Filter
{
    // data for each thread
    struct Thread_Data {
        unsigned start_y, end_y;
        Matrix* dst;
        Matrix* scratch;
        unsigned char* R;
        unsigned char* G;
        unsigned char* B;
        const double* weights;
        int radius;
        unsigned x_size;
        unsigned y_size;
    };


    namespace Gauss
    {
        void get_weights(int n, double *weights_out)
        {
            for (auto i{0}; i <= n; i++)
            {
                double x{static_cast<double>(i) * max_x / n};
                weights_out[i] = exp(-x * x * pi);
            }
        }
    }

   void* horizontal_blur_worker(void* arg) {
    // Extract thread data
    Thread_Data* tdata = static_cast<Thread_Data*>(arg);
    const auto R = tdata->R;
    const auto G = tdata->G;
    const auto B = tdata->B;
    auto x_size = tdata->x_size;
    auto y_size = tdata->y_size;
    auto radius = tdata->radius;
    const double* w = tdata->weights;
    Matrix& scratch = *tdata->scratch;
    //dst is not used in horizontal blur becuse we write to scratch 


    // Perform horizontal blur on assigned rows
    for (auto y = tdata->start_y; y < tdata->end_y; y++) {
        unsigned row_base = y * x_size;
        for (auto x = 0; x < x_size; x++) {
            double r = w[0] * R[row_base + x];
            double g = w[0] * G[row_base + x];
            double b = w[0] * B[row_base + x];
            double n = w[0];
            // Apply weights to neighboring pixels
            for (auto wi = 1; wi <= radius; wi++) {
                auto wc = w[wi];
                auto x2 = static_cast<int>(x) - wi;
                // Check bounds and accumulate weighted values
                if (x2 >= 0) {
                    r += wc * R[row_base + x2];
                    g += wc * G[row_base + x2];
                    b += wc * B[row_base + x2];
                    n += wc;
                }
                x2 = x + wi;
                // Check bounds and accumulate weighted values
                if (x2 < static_cast<int>(x_size)) {
                    r += wc * R[row_base + x2];
                    g += wc * G[row_base + x2];
                    b += wc * B[row_base + x2];
                    n += wc;
                }
            }
            // Normalize and store in scratch matrix
            scratch.r(x, y) = r / n;
            scratch.g(x, y) = g / n;
            scratch.b(x, y) = b / n;
        }
    }
    return nullptr;
}

void* vertical_blur_worker(void* arg) {
    // Extract thread data
    Thread_Data* tdata = static_cast<Thread_Data*>(arg);
    auto x_size = tdata->x_size;
    auto y_size = tdata->y_size;
    auto radius = tdata->radius;
    const double* w = tdata->weights;
    Matrix& scratch = *tdata->scratch;
    Matrix& dst = *tdata->dst;
    
    // Direct memory access for efficiency inatead of going through getters , needed pointer to modyfi the data for output
    auto R = const_cast<unsigned char*>(dst.get_R());
    auto G = const_cast<unsigned char*>(dst.get_G());
    auto B = const_cast<unsigned char*>(dst.get_B());
    // Perform vertical blur on assigned rows
    for (auto y = tdata->start_y; y < tdata->end_y; y++) {
        // Process each pixel in the row
        for (auto x = 0u; x < x_size; x++) {
            double r = w[0] * scratch.r(x, y);
            double g = w[0] * scratch.g(x, y);
            double b = w[0] * scratch.b(x, y);
            double n = w[0];
            // Apply weights to neighboring pixels

            for (auto wi = 1; wi <= radius; wi++) {
                auto wc = w[wi];
                auto y2 = static_cast<int>(y) - wi;
                if (y2 >= 0) {
                    // Check bounds and accumulate weighted values
                    r += wc * scratch.r(x, y2);
                    g += wc * scratch.g(x, y2);
                    b += wc * scratch.b(x, y2);
                    n += wc;
                }
                y2 = y + wi;
                if (y2 < static_cast<int>(y_size)) {
                    // Check bounds and accumulate weighted values
                    r += wc * scratch.r(x, y2);
                    g += wc * scratch.g(x, y2);
                    b += wc * scratch.b(x, y2);
                    n += wc;
                }
            }
            // Normalize and store in destination matrix

            R[y * x_size + x] = r / n;
            G[y * x_size + x] = g / n;
            B[y * x_size + x] = b / n;
        }
    }
    return nullptr;
}

Matrix blur(Matrix m, const int radius, const int threadscount) {

    //compute them only once
    //key optimization points are precomputing weights only once
    //and using a scratch matrix to avoid repeated allocations
    // use direct memory access via cached pointers like r, g, b arrays
    Matrix scratch{PPM::max_dimension};
    auto dst{m};
    // Precompute Gaussian weights
    double weights[Gauss::max_radius]{};
    Gauss::get_weights(radius, weights);

    // Get image dimensions 
    const auto x_size = dst.get_x_size();
    const auto y_size = dst.get_y_size();
    // Direct memory access for efficiency inatead of going through getters
    // xosnt beacuse we dont want to modify the original data
    unsigned char* R = const_cast<unsigned char*>(dst.get_R());
    unsigned char* G = const_cast<unsigned char*>(dst.get_G());
    unsigned char* B = const_cast<unsigned char*>(dst.get_B());

    pthread_t threads[threadscount];
    Thread_Data tdata[threadscount];
    

    // Divide work among threads
    unsigned slice = y_size / threadscount;
    for (int t = 0; t < threadscount; t++) {
        // Set up thread data
        // static_cast to avoid warnings
        tdata[t] = {static_cast<unsigned>(t * slice),
            static_cast<unsigned>((t == threadscount - 1) ? y_size : (t + 1) * slice)
            ,&dst, &scratch, R, G, B, weights, radius, x_size, y_size};
        pthread_create(&threads[t], nullptr, horizontal_blur_worker, &tdata[t]);
    }
    // Wait for all threads to finish
    for (int t = 0; t < threadscount; t++) pthread_join(threads[t], nullptr);

    for (int t = 0; t < threadscount; t++) {
        pthread_create(&threads[t], nullptr, vertical_blur_worker, &tdata[t]);
    }
    for (int t = 0; t < threadscount; t++) pthread_join(threads[t], nullptr);

    return dst;
}
};