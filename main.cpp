#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "Generator.h"
#include "Runner.h"

static std::string filePath(const std::string& dataDir, const Scenario& scenario, int index) {
    return dataDir + "/" + scenario.name + "_" + std::to_string(index) + ".txt";
}

int main(int argc, char* argv[]) {
    const int opsPerThread = argc > 1 ? std::stoi(argv[1]) : 500000;
    const int repeats = argc > 2 ? std::stoi(argv[2]) : 5;
    const int maxThreads = 3;
    const std::string dataDir = "data";

    std::filesystem::create_directories(dataDir);

    const std::vector<Scenario> scenarios = defaultScenarios();

    unsigned seed = 2026;
    for (const Scenario& scenario : scenarios) {
        for (int t = 1; t <= maxThreads; ++t) {
            generateFile(filePath(dataDir, scenario, t), scenario, opsPerThread, seed++);
        }
    }
    std::cout << "generated " << scenarios.size() * maxThreads << " files with "
              << opsPerThread << " ops each in "
              << std::filesystem::absolute(dataDir).string() << '\n'
              << "each measurement is repeated " << repeats << " times\n\n";

    std::vector<std::vector<double>> averages(scenarios.size(), std::vector<double>(maxThreads, 0.0));

    std::cout << std::fixed << std::setprecision(2);
    for (std::size_t s = 0; s < scenarios.size(); ++s) {
        std::vector<std::vector<Operation>> files;
        for (int t = 1; t <= maxThreads; ++t) {
            files.push_back(loadOperations(filePath(dataDir, scenarios[s], t)));
        }

        for (int threadCount = 1; threadCount <= maxThreads; ++threadCount) {
            std::vector<const std::vector<Operation>*> perThread;
            for (int t = 0; t < threadCount; ++t) {
                perThread.push_back(&files[t]);
            }

            std::cout << std::left << std::setw(10) << scenarios[s].name << std::right
                      << " threads=" << threadCount << ":";

            double sum = 0.0;
            for (int r = 0; r < repeats; ++r) {
                DataStruct ds;
                const double ms = runInThreads(ds, perThread);
                sum += ms;
                std::cout << ' ' << std::setw(8) << ms;
            }
            averages[s][threadCount - 1] = sum / repeats;
            std::cout << "   avg=" << std::setw(8) << averages[s][threadCount - 1] << " ms\n";
        }
        std::cout << '\n';
    }

    std::cout << "average time, ms (" << opsPerThread << " ops per thread)\n";
    std::cout << std::setw(12) << "";
    for (int t = 1; t <= maxThreads; ++t) {
        std::cout << std::setw(12) << (std::to_string(t) + (t == 1 ? " thread" : " threads"));
    }
    std::cout << '\n';
    for (std::size_t s = 0; s < scenarios.size(); ++s) {
        std::cout << std::left << std::setw(12) << scenarios[s].name << std::right;
        for (int t = 0; t < maxThreads; ++t) {
            std::cout << std::setw(12) << averages[s][t];
        }
        std::cout << '\n';
    }
    return 0;
}
