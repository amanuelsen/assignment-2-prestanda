/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>
#include <vector>
#include <pthread.h>

namespace Filter
{
    struct BlurJob
    {
        int radius;
        int start_row;
        int end_row;
        Matrix* src;
        Matrix* dst;
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

    void* blur_horizontal_worker(void* arg)
    {
        BlurJob* job = static_cast<BlurJob*>(arg);
        Matrix* src = job->src;
        Matrix* dst = job->dst;
        int radius = job->radius;

        double w[Gauss::max_radius];
        Gauss::get_weights(radius, w);

        for (int y = job->start_row; y < job->end_row; ++y)
        {
            for (unsigned x = 0; x < src->get_x_size(); ++x)
            {
                double r = w[0] * src->r(x, y);
                double g = w[0] * src->g(x, y);
                double b = w[0] * src->b(x, y);
                double n = w[0];

                for (int wi = 1; wi <= radius; ++wi)
                {
                    int x2 = x - wi;
                    if (x2 >= 0) {
                        r += w[wi] * src->r(x2, y);
                        g += w[wi] * src->g(x2, y);
                        b += w[wi] * src->b(x2, y);
                        n += w[wi];
                    }
                    x2 = x + wi;
                    if (x2 < (int)src->get_x_size()) {
                        r += w[wi] * src->r(x2, y);
                        g += w[wi] * src->g(x2, y);
                        b += w[wi] * src->b(x2, y);
                        n += w[wi];
                    }
                }

                dst->r(x, y) = static_cast<unsigned char>(r / n);
                dst->g(x, y) = static_cast<unsigned char>(g / n);
                dst->b(x, y) = static_cast<unsigned char>(b / n);
            }
        }

        return nullptr;
    }

    Matrix blur(Matrix m, const int radius, const int threadscount)
    {
        Matrix scratch{PPM::max_dimension};
        auto dst = m;

        int rows = dst.get_y_size();
        int rows_per_thread = rows / threadscount;
        int remaining = rows % threadscount;
        int current_row = 0;

        std::vector<pthread_t> threads(threadscount);
        std::vector<BlurJob> jobs(threadscount);

        // Launch threads for horizontal pass only
        for (int i = 0; i < threadscount; ++i)
        {
            int end_row = current_row + rows_per_thread + (i < remaining ? 1 : 0);
            jobs[i] = {radius, current_row, end_row, &dst, &scratch};
            if (pthread_create(&threads[i], nullptr, blur_horizontal_worker, &jobs[i]) != 0)
                throw std::runtime_error("Failed to create thread");
            current_row = end_row;
        }

        // Wait for threads to finish horizontal pass
        for (int i = 0; i < threadscount; ++i)
            pthread_join(threads[i], nullptr);

        // Vertical pass runs sequentially on scratch -> dst
        for (unsigned x = 0; x < dst.get_x_size(); ++x)
        {
            for (unsigned y = 0; y < dst.get_y_size(); ++y)
            {
                double w[Gauss::max_radius];
                Gauss::get_weights(radius, w);

                double r = w[0] * scratch.r(x, y);
                double g = w[0] * scratch.g(x, y);
                double b = w[0] * scratch.b(x, y);
                double n = w[0];

                for (int wi = 1; wi <= radius; ++wi)
                {
                    int y2 = y - wi;
                    if (y2 >= 0)
                    {
                        r += w[wi] * scratch.r(x, y2);
                        g += w[wi] * scratch.g(x, y2);
                        b += w[wi] * scratch.b(x, y2);
                        n += w[wi];
                    }
                    y2 = y + wi;
                    if (y2 < (int)dst.get_y_size())
                    {
                        r += w[wi] * scratch.r(x, y2);
                        g += w[wi] * scratch.g(x, y2);
                        b += w[wi] * scratch.b(x, y2);
                        n += w[wi];
                    }
                }

                dst.r(x, y) = static_cast<unsigned char>(r / n);
                dst.g(x, y) = static_cast<unsigned char>(g / n);
                dst.b(x, y) = static_cast<unsigned char>(b / n);
            }
        }

        return dst;
    }
};
        // Vertical pass runs sequentially on scratch -> dst
       
        

 