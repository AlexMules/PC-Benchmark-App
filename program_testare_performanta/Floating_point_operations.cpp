#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <atomic>

using namespace std;

// --- Structura Rezultatelor ---
struct BenchmarkResult {
    double gflops;
    double latency_ns;
    double duration;
};

// --- Functia de prevenire a optimizarii ---
template <typename T>
void doNotOptimize(T const& value) {
    volatile const T* p = &value;
    (void)*p;
    atomic_signal_fence(memory_order_seq_cst);
}

// --- Mandelbrot Benchmark ---
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

    const double OPS_PER_ITERATION = 8.0; // 4 MUL + 3 ADD + 1 SUB

    struct PassResult {
        double durationSeconds;
        unsigned long long totalIterations;
    };

    PassResult runPass() {
        double dx = (X_MAX - X_MIN) / WIDTH;
        double dy = (Y_MAX - Y_MIN) / HEIGHT;
        unsigned long long iterCount = 0;

        atomic_thread_fence(memory_order_seq_cst);
        auto start = chrono::steady_clock::now();

        for (int j = 0; j < HEIGHT; ++j) {
            double y = Y_MAX - j * dy;
            for (int i = 0; i < WIDTH; ++i) {
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
    BenchmarkResult runBenchmark() {
        cout << "--- BENCHMARK FPU START (Numere Reale) ---" << endl;
        cout << "Configurare: " << NUM_SAMPLES << " Sample-uri, " << MAX_ITER << " Max Iteratii." << endl;
        cout << "Se executa " << WARMUP_PASSES << " rulari de incalzire..." << endl;

        auto startTotal = chrono::steady_clock::now();

        // --- WARMUP ---
        for (int i = 0; i < WARMUP_PASSES; ++i) {
            runPass();
            cout << "." << flush;
        }
        cout << endl;
        cout << "Se executa testele de performanta..." << endl;

        vector<double> gflopsSamples;
        gflopsSamples.reserve(NUM_SAMPLES);

        for (int i = 0; i < NUM_SAMPLES; ++i) {
            PassResult res = runPass();
            double totalFlops = res.totalIterations * OPS_PER_ITERATION;
            double currentGflops = (totalFlops / res.durationSeconds) / 1e9;
            gflopsSamples.push_back(currentGflops);

            if ((i + 1) % 10 == 0)
                cout << (i + 1) << " " << flush;
            else
                cout << "." << flush;
        }

        sort(gflopsSamples.begin(), gflopsSamples.end());
        double medianGflops = gflopsSamples[NUM_SAMPLES / 2];
        double latencyNs = 1.0 / medianGflops; // ns/FLOP

        auto endTotal = chrono::steady_clock::now();
        chrono::duration<double> totalElapsed = endTotal - startTotal;

        BenchmarkResult result;
        result.gflops = medianGflops;
        result.latency_ns = latencyNs;
        result.duration = totalElapsed.count();
        return result;
    }
};

void printFinalResults(const BenchmarkResult& result) {
    cout << "==================================================" << endl;
    cout << "             REZULTATE PERFORMANTA FPU            " << endl;
    cout << "==================================================" << endl;
    cout << fixed << setprecision(3);

    cout << " SCOR (Mediana):         " << result.gflops << " GFLOPS" << endl;
    cout << " Latenta medie:          " << result.latency_ns << " ns/op" << endl;
    cout << " Durata totala test:     " << result.duration << " secunde" << endl;

    cout << "==================================================" << endl << endl;
}

int main() {
    MandelbrotBenchmark benchmark;
    BenchmarkResult final_result = benchmark.runBenchmark();

    printFinalResults(final_result);

    cout << "Apasa Enter pentru a iesi...";
    cin.get();
    return 0;
}
