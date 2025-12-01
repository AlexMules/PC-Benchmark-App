#pragma once

#define BENCHMARK __declspec(dllexport)

#include <cstddef>

extern "C" {
    struct BenchmarkResult {
        double sequential;
        int stride_count;
        int strides[3];
        double strided_results[3];
        double random;
    };

    BENCHMARK void RunBenchmark(size_t blockSize, BenchmarkResult* result);
}
