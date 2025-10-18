/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "vector.hpp"
#include <iostream>
#include <cmath>
#include <vector>

// used for SIMD optimization
#include <immintrin.h>

Vector::Vector()
    : size{0}, data{nullptr}
{
}

Vector::~Vector()
{
    if (data)
    {
        delete[] data;
    }

    size = 0;
}

Vector::Vector(unsigned size)
    : size{size}, data{new double[size]}
{
}

Vector::Vector(unsigned size, double *data)
    : size{size}, data{data}
{
}

Vector::Vector(const Vector &other)
    : Vector{other.size}
{
    for (auto i{0}; i < size; i++)
    {
        data[i] = other.data[i];
    }
}

unsigned Vector::get_size() const
{
    return size;
}

double *Vector::get_data()
{
    return data;
}

double Vector::operator[](unsigned i) const
{
    return data[i];
}

double &Vector::operator[](unsigned i)
{
    return data[i];
}

double Vector::mean() const
{
    double sum{0};

    for (auto i{0}; i < size; i++)
    {
        sum += data[i];
    }

    return sum / static_cast<double>(size);
}

double Vector::magnitude() const
{
    auto dot_prod{dot(*this)};
    return std::sqrt(dot_prod);
}

Vector Vector::operator/(double div)
{
    auto result{*this};

    for (auto i{0}; i < size; i++)
    {
        result[i] /= div;
    }

    return result;
}

Vector Vector::operator-(double sub)
{
    auto result{*this};

    for (auto i{0}; i < size; i++)
    {
        result[i] -= sub;
    }

    return result;
}

// SIMD optimization for dot function
double Vector::dot(const Vector& rhs) const
{
    double result = 0.0;
    int i = 0;

    // use AVX to process 4 doubles at a time
    __m256d acc = _mm256_setzero_pd();

    // increments of 4
    for (; i + 4 <= size; i += 4)
    {
        __m256d a = _mm256_loadu_pd(&data[i]);
        __m256d b = _mm256_loadu_pd(&rhs.data[i]);
        __m256d prod = _mm256_mul_pd(a, b);
        acc = _mm256_add_pd(acc, prod);
    }

    // horizontally add the 4 elements in acc
    alignas(32) double temp[4];
    _mm256_store_pd(temp, acc);
    result = temp[0] + temp[1] + temp[2] + temp[3];

    // handle remaining elements if any exists (outside the step of 4)
    for (; i < size; ++i)
    {
        result += data[i] * rhs.data[i];
    }

    return result;
} 