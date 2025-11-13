#pragma once

#ifdef BENCHMARKDLL_EXPORTS
#define BENCHMARK __declspec(dllexport)
#else
#define BENCHMARK __declspec(dllimport)
#endif

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
