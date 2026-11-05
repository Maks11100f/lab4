#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "DataStruct.h"

static void singleThreadCheck() {
    DataStruct ds;
    std::cout << "initial:          " << std::string(ds) << '\n';

    ds.set(0, 13);
    ds.set(1, 42);
    std::cout << "after set(0, 13), set(1, 42): " << std::string(ds) << '\n';
    std::cout << "get(0) = " << ds.get(0) << ", get(1) = " << ds.get(1) << '\n';
}

static void multiThreadStress(int threadCount, int opsPerThread) {
    DataStruct ds;

    auto worker = [&ds, opsPerThread](unsigned seed) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> opDist(0, 4);
        std::size_t sink = 0;  // щоб компілятор не викинув читання

        for (int k = 0; k < opsPerThread; ++k) {
            switch (opDist(rng)) {
                case 0: sink += ds.get(0); break;
                case 1: ds.set(0, k); break;
                case 2: sink += ds.get(1); break;
                case 3: ds.set(1, k); break;
                case 4: sink += std::string(ds).size(); break;
            }
        }
        volatile std::size_t keep = sink;
        (void)keep;
    };

    std::thread threads[3];
    for (int t = 0; t < threadCount; ++t) {
        threads[t] = std::thread(worker, 1000u + t);
    }
    for (int t = 0; t < threadCount; ++t) {
        threads[t].join();
    }

    std::cout << "stress " << threadCount << " thread(s) x " << opsPerThread
              << " ops: OK, final " << std::string(ds) << '\n';
}

int main() {
    singleThreadCheck();
    for (int t = 1; t <= 3; ++t) {
        multiThreadStress(t, 200000);
    }
    return 0;
}
