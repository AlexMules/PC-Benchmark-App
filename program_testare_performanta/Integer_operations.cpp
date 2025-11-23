// Integer_operations.cpp
// Compile (MSVC):
// cl /O2 /Ob2 /favor:AMD64 /LD Integer_operations.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>
#include <atomic>
#include <intrin.h>

using namespace std;

template <typename T>
void doNotOptimize(T const& value) {
    volatile const T* p = &value;
    (void)*p;
    atomic_signal_fence(memory_order_seq_cst);
}

class IntegerOperationsBenchmark {
private:
    const size_t BUFFER_SIZE = 1024;
    const size_t ITERATIONS_PER_PASS = 200000;
    const int NUM_SAMPLES = 202;
    const int WARMUP_PASSES = 5;

    const double OPS_PER_ELEMENT = 11.0;
    /*
     * OPS_PER_ELEMENT = 11 operations:
     *   3 ADDs
     *   1 MUL
     *   1 SUB
     *   3 XORs
     *   1 NOT
     *   1 ROL
     *   1 AND
     */

    const double OPS_PER_PASS = static_cast<double>(BUFFER_SIZE) * OPS_PER_ELEMENT
        * static_cast<double>(ITERATIONS_PER_PASS);

    vector<int> buffer_1;
    vector<int> buffer_2;
    mt19937 generator; // random numbers generator
    uniform_int_distribution<int> dis; // returns a random integer in a range

    static inline int RotateLeft(int value, int shift) {
        return static_cast<int>(_rotl(static_cast<unsigned int>(value), shift & 0x1F));
    }

    void initializeBuffers() {
        for (size_t i = 0; i < BUFFER_SIZE; i++) {
            buffer_1[i] = dis(generator);
            buffer_2[i] = dis(generator);
        }
    }

    double runBenchmarkPass() {
        initializeBuffers();

        int val_add = dis(generator);
        int val_mul = (dis(generator) % 100) | 1;
        int val_xor = -889275714;
        int resultSum = 0;

        atomic_thread_fence(memory_order_seq_cst);
        auto start = chrono::steady_clock::now();

        for (size_t it = 0; it < ITERATIONS_PER_PASS; it++) {
            for (size_t i = 0; i < BUFFER_SIZE; i++) {
                int val_1 = buffer_1[i];
                int val_2 = buffer_2[i];

                int r1 = (val_1 + val_add) * val_mul - static_cast<int>(it);
                int r2 = ~(val_1 ^ val_xor);
                int r3 = RotateLeft(val_2, val_1 & 0x1F);
                int r4 = (val_2 + static_cast<int>(i)) & 0xFFFF;

                int result = r1 ^ r2 ^ r3 ^ r4;

                buffer_1[i] = result;
                resultSum += result;
            }

            val_mul += 2;
        }

        doNotOptimize(resultSum);
        doNotOptimize(buffer_1[0]);
        doNotOptimize(buffer_2[0]);

        atomic_thread_fence(memory_order_seq_cst);
        auto end = chrono::steady_clock::now();

        chrono::duration<double> duration = end - start;
        return duration.count();
    }

public:
    IntegerOperationsBenchmark() // member initializer list
        : buffer_1(BUFFER_SIZE)
        , buffer_2(BUFFER_SIZE)
        , generator(random_device{}()) // random seed
        , dis(-10000, 10000) // generates a random integer between -10000 and 10000
    {
    }

    // Returns median GOps
    double runBenchmark() {
        vector<double> scores;
        scores.reserve(NUM_SAMPLES);

        // warmup runs
        for (int w = 0; w < WARMUP_PASSES; w++) {
            runBenchmarkPass();
        }

        // benchmark runs
        for (int i = 0; i < NUM_SAMPLES; i++) {
            double time = runBenchmarkPass();
            double gops = (OPS_PER_PASS / time) / 1e9;
            scores.push_back(gops);
        }

        sort(scores.begin(), scores.end());
        double median_gops = scores[NUM_SAMPLES / 2];

        return median_gops;
    }
};

// Structure for benchmark results
struct BenchmarkResult {
    double gops;        // Giga Operations per Second
    double latency_ns;  // nanoseconds per operation
    double duration;    // benchmark duration in seconds
};

// Exported functions for DLL
extern "C" {
    // Returns full benchmark results
    __declspec(dllexport) BenchmarkResult runIntegerBenchmark() {
        auto startTime = chrono::steady_clock::now();

        IntegerOperationsBenchmark benchmark;
        double median_gops = benchmark.runBenchmark();

        auto endTime = chrono::steady_clock::now();
        double duration = chrono::duration<double>(endTime - startTime).count();

        BenchmarkResult result;
        result.gops = median_gops;
        result.latency_ns = 1.0 / median_gops; // nanoseconds per operation
        result.duration = duration;

        return result;
    }

    __declspec(dllexport) double runIntegerBenchmarkSimple() {
        IntegerOperationsBenchmark benchmark;
        return benchmark.runBenchmark();
    }
}