// Floating_point_operations.cpp
// Compile (MSVC):
// cl /O2 /Ob2 /favor:AMD64 /LD Floating_point_operations.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <atomic>

using namespace std;

template <typename T>
void doNotOptimize(T const& value) {
    volatile const T* p = &value;
    (void)*p;
    atomic_signal_fence(memory_order_seq_cst);
}


struct BenchmarkResult {
    double gflops;  
    double latency_ns; 
    double duration;   
};


class MandelbrotBenchmark {
private:
    const int WIDTH = 2048;
    const int HEIGHT = 2048;
    const int MAX_ITER = 2500;

    const int NUM_SAMPLES = 11;
    const int WARMUP_PASSES = 5;

    const double X_MIN = -0.7463;
    const double X_MAX = -0.7400;
    const double Y_MIN = 0.1102;
    const double Y_MAX = 0.1137;

    const double OPS_PER_ITERATION = 8.0;

    struct PassResult {
        double durationSeconds;
        unsigned long long totalIterations;
    };

    PassResult runBenchmarkPass() {
        double dx = (X_MAX - X_MIN) / WIDTH;
        double dy = (Y_MAX - Y_MIN) / HEIGHT;
        unsigned long long iterCount = 0;

        atomic_thread_fence(memory_order_seq_cst);
        auto start = chrono::steady_clock::now();

        for (int j = 0; j < HEIGHT; j++) {
            double y = Y_MAX - j * dy;
            for (int i = 0; i < WIDTH; i++) {
                double x = X_MIN + i * dx;
                double u = 0.0, v = 0.0;
                double u2 = 0.0, v2 = 0.0;
                int k = 0;

                while (k < MAX_ITER && (u2 + v2 < 4.0)) {
                    v = 2 * u * v + y;
                    u = u2 - v2 + x;
                    u2 = u * u;
                    v2 = v * v;
                    k++;
                }
                iterCount += k;
            }
        }

        doNotOptimize(iterCount);
        atomic_thread_fence(memory_order_seq_cst);

        auto end = chrono::steady_clock::now();
        chrono::duration<double> diff = end - start;
        return { diff.count(), iterCount };
    }

public:
    double runBenchmark() {
        vector<double> scores;
        scores.reserve(NUM_SAMPLES);

        for (int w = 0; w < WARMUP_PASSES; w++) {
            runBenchmarkPass();
        }

        for (int i = 0; i < NUM_SAMPLES; i++) {
            PassResult res = runBenchmarkPass();
            double totalFlops = res.totalIterations * OPS_PER_ITERATION;
            double currentGflops = (totalFlops / res.durationSeconds) / 1e9;
            scores.push_back(currentGflops);
        }

        sort(scores.begin(), scores.end());
        return scores[NUM_SAMPLES / 2];
    }

    MandelbrotBenchmark() {}
};


extern "C" {
    __declspec(dllexport) BenchmarkResult runMandelbrotBenchmark() {
        MandelbrotBenchmark benchmark;

        auto startTime = chrono::steady_clock::now();
        double median_gflops = benchmark.runBenchmark();
        auto endTime = chrono::steady_clock::now();
        double duration = chrono::duration<double>(endTime - startTime).count();

        BenchmarkResult result;
        result.gflops = median_gflops;
        result.latency_ns = 1.0 / median_gflops;
        result.duration = duration;

        return result;
    }
}