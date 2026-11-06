#include <filesystem>
#include <iostream>
#include <string>

#include "Generator.h"

int main(int argc, char* argv[]) {
    const int opsPerThread = argc > 1 ? std::stoi(argv[1]) : 500000;
    const int maxThreads = 3;
    const std::string dataDir = "data";

    std::filesystem::create_directories(dataDir);

    unsigned seed = 2026;
    for (const Scenario& scenario : defaultScenarios()) {
        for (int t = 1; t <= maxThreads; ++t) {
            const std::string path = dataDir + "/" + scenario.name + "_" + std::to_string(t) + ".txt";
            generateFile(path, scenario, opsPerThread, seed++);
            std::cout << "generated " << path << " (" << opsPerThread << " ops, "
                      << std::filesystem::file_size(path) / 1024 << " KB)\n";
        }
    }

    std::cout << "files are in " << std::filesystem::absolute(dataDir).string() << '\n';
    return 0;
}
