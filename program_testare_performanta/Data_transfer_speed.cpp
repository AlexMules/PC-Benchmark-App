// Data_transfer_speed.cpp
// Compile (MSVC): cl /O2 /EHsc Data_transfer_speed.cpp

#define BENCHMARKDLL_EXPORTS
#include "Data_transfer_speed.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <random>
#include <iomanip>
#include <cstring>
#include <malloc.h>
#include <intrin.h>

using namespace std;

// previne optimizarile efectuate de compilator
template<typename T>
inline void doNotOptimize(T& value) {
    volatile T* vp = &value;
    (void)*vp;
    _ReadWriteBarrier();
}

// clasa PrecisionTimer pentru masurare
class PrecisionTimer {
    chrono::high_resolution_clock::time_point startTime, endTime;
public:
    void start() { 
        startTime = chrono::high_resolution_clock::now(); 
    }

    void stop() { 
        endTime = chrono::high_resolution_clock::now(); 
    }

    double getElapsedNanoseconds() const {
        return chrono::duration<double, nano>(endTime - startTime).count();
    }
};

const size_t KB = 1024;
const size_t MB = 1024 * KB;
const size_t MAX_BUF = 512ULL * 1024ULL * 1024ULL; // MAX_BUF = 512 MB

// folosesc flushCache intre iteratii
void flushCache(const size_t blockSize) {
    size_t flushSize = min(blockSize * 2, MAX_BUF);
    static vector<char> flushBuffer(MAX_BUF, 1);

    // scriere in flushBuffer
    memset(flushBuffer.data(), 1, flushSize);

    // citire fiecare cache line
    volatile char dummy = 0;
    for (size_t i = 0; i < flushSize; i += 64) { // dim. cache line = 64 bytes
        dummy += flushBuffer[i];
    }
    doNotOptimize(dummy);
}


// parametri adaptivi in functie de dimensiunea blocului de date
int getIterations(const size_t blockSize) {
    if (blockSize <= 16 * MB) {
        return 100;
    }

    if (blockSize <= 512 * MB) {
        return 30;
    }

    return 10;
}

int getWarmupRuns(const size_t blockSize) {
    if (blockSize <= 16 * MB) {
        return 5;
    }

    if (blockSize <= 512 * MB) {
        return 3;
    }

    return 2;
}

vector<int> getStrides(const size_t blockSize) {
    if (blockSize <= 16 * MB) {
        return { 4, 8, 16 };
    }

    if (blockSize <= 512 * MB) {
        return { 16, 64, 256 };
    }

    return {}; // pentru blocuri mari (de ex. 1 GB), nu se face testul de strided access
}


void printError() {
    cout << "Memory allocation error! Aborting tests...\n";      
}

void computeSpeed(const PrecisionTimer& timer, const size_t bytes, vector<double>& speeds) {
    double seconds = timer.getElapsedNanoseconds() / 1e9;
    double mbs = bytes / (1024.0 * 1024.0);
    speeds.push_back(mbs / seconds);
}

double computeMedian(vector<double> speeds) {
    sort(speeds.begin(), speeds.end());
    return speeds[speeds.size() / 2]; // valoarea mediana
}

// clasa pentru benchmark
class DataTransferBenchmark {
public:
    double sequentialAccess(const size_t blockSize) {
        char* src = (char*)_aligned_malloc(blockSize, 64);
        char* dst = (char*)_aligned_malloc(blockSize, 64);

        if (!src || !dst) {
            _aligned_free(src);
            _aligned_free(dst);
            printError();
            exit(1);
        }

        // initializare date
        memset(src, 1, blockSize);

        int warmups = getWarmupRuns(blockSize);
        int iterations = getIterations(blockSize);
        vector<double> speeds;
        speeds.reserve(iterations);
        PrecisionTimer timer;

        // warmup pentru a evita "cold cache"
        for (int w = 0; w < warmups; w++) {
            memcpy(dst, src, blockSize);
            doNotOptimize(dst[0]);
        }

        // masurare efectiva
        for (int i = 0; i < iterations; i++) {
            flushCache(blockSize);

            timer.start();
            memcpy(dst, src, blockSize);
            timer.stop();

            doNotOptimize(dst[0]);

            computeSpeed(timer, blockSize, speeds);
        }

        _aligned_free(src);
        _aligned_free(dst);

        return computeMedian(speeds);
    }

    double stridedAccess(const size_t blockSize, const int stride) {
        size_t numInts = blockSize / sizeof(int);

        int* src = (int*)_aligned_malloc(blockSize, 64);
        int* dst = (int*)_aligned_malloc(blockSize, 64);

        if (!src || !dst) {
            _aligned_free(src);
            _aligned_free(dst);
            printError();
            exit(1);
        }

        // initializare date
        iota(src, src + numInts, 0);

        int warmups = getWarmupRuns(blockSize);
        int iterations = getIterations(blockSize);
        vector<double> speeds;
        speeds.reserve(iterations);
        PrecisionTimer timer;

        // warmup pentru a evita "cold cache"
        for (int w = 0; w < warmups; w++) {
            volatile int dummy = 0;
            for (size_t i = 0; i < numInts; i += stride) {
                dst[i] = src[i];
                dummy += dst[i]; 
            }
            doNotOptimize(dummy);
        }

        // masurare efectiva
        for (int it = 0; it < iterations; it++) {
            flushCache(blockSize);

            volatile int dummy = 0; // dummy folosit pentru a evita optimizare compilator
            timer.start();
            for (size_t i = 0; i < numInts; i += stride) {
                dst[i] = src[i];
                dummy += dst[i];
            }
            timer.stop();

            doNotOptimize(dummy);

            size_t accessed = (numInts + stride - 1) / stride;
            double bytes = accessed * sizeof(int);
            computeSpeed(timer, bytes, speeds);
        }

        _aligned_free(src);
        _aligned_free(dst);

        return computeMedian(speeds);
    }

    double randomAccess(const size_t blockSize) {
        size_t numInts = blockSize / sizeof(int);

        int* src = (int*)_aligned_malloc(blockSize, 64);
        int* dst = (int*)_aligned_malloc(blockSize, 64);

        if (!src || !dst) {
            _aligned_free(src);
            _aligned_free(dst);
            printError();
            exit(1);
        }

        // initializare date
        iota(src, src + numInts, 0);

        size_t accessCount = min(numInts, max<size_t>(10000, blockSize / 1024));
        int warmups = getWarmupRuns(blockSize);
        int iterations = getIterations(blockSize);
        vector<double> speeds;
        speeds.reserve(iterations);
        PrecisionTimer timer;

        // generare indici pentru acces random
        mt19937 g(42); // folosire seed fix pentru benchmarking consistent

        // warmup pentru a evita "cold cache"
        for (int w = 0; w < warmups; w++) {
            vector<size_t> indices(numInts);
            iota(indices.begin(), indices.end(), 0);
            shuffle(indices.begin(), indices.end(), g);

            volatile int dummy = 0;
            for (size_t i = 0; i < min(accessCount, size_t(1000)); i++)
                dummy += (dst[indices[i]] = src[indices[i]]);
            doNotOptimize(dummy);
        }

        // masurare efectiva
        for (int it = 0; it < iterations; it++) {
            vector<size_t> indices(numInts);
            iota(indices.begin(), indices.end(), 0);
            shuffle(indices.begin(), indices.end(), g);

            flushCache(blockSize);

            volatile int dummy = 0;
            timer.start();
            for (size_t i = 0; i < accessCount; i++)
                dummy += (dst[indices[i]] = src[indices[i]]);
            timer.stop();

            doNotOptimize(dummy);

            double bytes = accessCount * sizeof(int);
            computeSpeed(timer, bytes, speeds);
        }

        _aligned_free(src);
        _aligned_free(dst);

        return computeMedian(speeds);
    }
};

extern "C" {
    BENCHMARK void RunBenchmark(size_t blockSize, BenchmarkResult* result) {
        if (!result) {
            return;
        }

        DataTransferBenchmark bench;

        result->sequential = bench.sequentialAccess(blockSize);

        auto strides = getStrides(blockSize);
        result->stride_count = static_cast<int>(strides.size());

        for (size_t i = 0; i < strides.size(); i++) {
            result->strides[i] = strides[i];
            result->strided_results[i] = bench.stridedAccess(blockSize, strides[i]);
        }

        result->random = bench.randomAccess(blockSize);
    }
}