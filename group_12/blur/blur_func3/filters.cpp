/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>

namespace Filter
{

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

    Matrix blur(Matrix m, const int radius)
    {
        Matrix scratch{PPM::max_dimension};
        auto dst{m};
         // Precompute weights once and not within the loops
        double w[Gauss::max_radius]{};
        Gauss::get_weights(radius, w);
        // cache sizes and pointers to avoid multiple calls to getters in inner loops like dst.r(x,y) does 
        const auto x_size=dst.get_x_size();
        const auto y_size=dst.get_y_size();
        // avoid multiple calls to dst.get_R() in inner loops and similar for G and B
        // we are going to modify dst so we need to cast away constness her by using const_cast that removes the const qualifier from a pointer
        unsigned char* R=const_cast<unsigned char*>(dst.get_R());
        unsigned char* G=const_cast<unsigned char*>(dst.get_G());
        unsigned char* B=const_cast<unsigned char*>(dst.get_B());
        //Avoid repeated getter function calls in the inner loops
        //Improve performance by enabling direct memory access to pixel data

        // First pass: horizontal blur for better cache locality according to how array is stored in memory
        for (auto y{0}; y < y_size; y++)
        {
            for (auto x{0}; x < x_size; x++)
            {
                // calculate row base index once per row to avoid repeated multiplication in inner loop
                unsigned row_base=y*x_size;
           

                // unsigned char Matrix::r(unsigned x, unsigned y) const
                // {
                //     return R[y * x_size + x];
                // }
               // avoid calling dst.r(x,y) multiple times in inner loops and instead use cached pointer and sizes
                // same for g and b
                // also avoid multiple calls to w[wi]
                auto r{w[0] * R[row_base + x]}, g{w[0] * G[row_base + x]}, b{w[0] * B[row_base + x]}, n{w[0]};

                for (auto wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto x2{x - wi};
                    
                    if (x2 >= 0)
                    {
                        // same as dst.r(x2,y) but using cached pointer and sizes
                        r += wc * R[row_base + x2];
                        g += wc * G[row_base + x2];
                        b += wc * B[row_base + x2];
                        n += wc;
                    }
                    x2 = x + wi;
                    if (x2 < x_size)
                    {
                        // same as dst.r(x2,y) but using cached pointer and sizes so we do the matrix lookup only once per channel
                        r += wc * R[row_base + x2];
                        g += wc * G[row_base + x2];
                        b += wc * B[row_base + x2];
                        n += wc;
                    }
                }
                // same as scratch.r(x,y) but using cached pointer and sizes
                scratch.r(x, y) = r / n;
                scratch.g(x, y) = g / n;
                scratch.b(x, y) = b / n;
            }
        }

        for (auto y{0}; y < y_size; y++)
        {
            for (auto x{0}; x < x_size; x++)
            {
              

                auto r{w[0] * scratch.r(x, y)}, g{w[0] * scratch.g(x, y)}, b{w[0] * scratch.b(x, y)}, n{w[0]};

                for (auto wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto y2{y - wi};
                    if (y2 >= 0)
                    {
                        // same as dst.r(x,y2) but using cached pointer and sizes

                        r += wc * scratch.r(x, y2);
                        g += wc * scratch.g(x, y2);
                        b += wc * scratch.b(x, y2);
                        n += wc;
                    }
                    y2 = y + wi;
                    if (y2 < y_size)
                    {
                         // same thing here, avoid calling dst.r(x,y) multiple times in inner loops and instead use cached pointer and sizes

                         r += wc * scratch.r(x, y2);
                         g += wc * scratch.g(x, y2);
                         b += wc * scratch.b(x, y2);
                         n += wc;
                    }
                }
                // same thing here, avoid calling dst.r(x,y) multiple times in inner loops and instead use cached pointer and sizes
                R[y * x_size + x] = r / n;
                G[y * x_size + x] = g / n;
                B[y * x_size + x] = b / n;
            }
        }

        return dst;
    }

}
