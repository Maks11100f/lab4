#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "Generator.h"
#include "Runner.h"

int main(int argc, char* argv[]) {
    const int opsPerThread = argc > 1 ? std::stoi(argv[1]) : 500000;
    const int maxThreads = 3;
    const std::string dataDir = "data";

    std::filesystem::create_directories(dataDir);

    const std::vector<Scenario> scenarios = defaultScenarios();

    unsigned seed = 2026;
    for (const Scenario& scenario : scenarios) {
        for (int t = 1; t <= maxThreads; ++t) {
            const std::string path = dataDir + "/" + scenario.name + "_" + std::to_string(t) + ".txt";
            generateFile(path, scenario, opsPerThread, seed++);
        }
    }
    std::cout << "generated " << scenarios.size() * maxThreads << " files with "
              << opsPerThread << " ops each in "
              << std::filesystem::absolute(dataDir).string() << "\n\n";

    for (const Scenario& scenario : scenarios) {
        std::vector<std::vector<Operation>> files;
        for (int t = 1; t <= maxThreads; ++t) {
            files.push_back(loadOperations(dataDir + "/" + scenario.name + "_" + std::to_string(t) + ".txt"));
        }

        for (int threadCount = 1; threadCount <= maxThreads; ++threadCount) {
            std::vector<const std::vector<Operation>*> perThread;
            for (int t = 0; t < threadCount; ++t) {
                perThread.push_back(&files[t]);
            }

            DataStruct ds;
            const double ms = runInThreads(ds, perThread);

            std::cout << std::left << std::setw(10) << scenario.name
                      << " threads=" << threadCount
                      << "  time=" << std::right << std::fixed << std::setprecision(2)
                      << std::setw(9) << ms << " ms"
                      << "  final=" << std::string(ds) << '\n';
        }
        std::cout << '\n';
    }
    return 0;
}
