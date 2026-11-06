#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "Generator.h"
#include "Runner.h"

using namespace std;

const int maxThreads = 3;
const string dataDir = "data";

string filePath(const Scenario &scenario, int index)
{
    return format("{}/{}_{}.txt", dataDir, scenario.name, index);
}

int main(int argc, char *argv[])
{
    int opsPerThread = argc > 1 ? atoi(argv[1]) : 500000;
    int repeats = argc > 2 ? atoi(argv[2]) : 5;

    filesystem::create_directories(dataDir);
    auto scenarios = defaultScenarios();

    unsigned seed = 2026;
    for (const auto &scenario : scenarios)
        for (int t = 1; t <= maxThreads; ++t)
            generateFile(filePath(scenario, t), scenario, opsPerThread, seed++);
    cout << format("generated {} files with {} ops each in {}",
                   scenarios.size() * maxThreads, opsPerThread, filesystem::absolute(dataDir).string())
         << endl;
    cout << format("each measurement is repeated {} times", repeats) << endl
         << endl;

    vector<vector<double>> averages(scenarios.size(), vector<double>(maxThreads));
    for (size_t s = 0; s < scenarios.size(); ++s)
    {
        vector<vector<Operation>> files;
        for (int t = 1; t <= maxThreads; ++t)
            files.push_back(loadOperations(filePath(scenarios[s], t)));

        for (int threadCount = 1; threadCount <= maxThreads; ++threadCount)
        {
            cout << format("{:<10} threads={}:", scenarios[s].name, threadCount);
            double sum = 0;
            for (int r = 0; r < repeats; ++r)
            {
                DataStruct ds;
                double ms = runInThreads(ds, files, threadCount);
                sum += ms;
                cout << format(" {:8.2f}", ms);
            }
            averages[s][threadCount - 1] = sum / repeats;
            cout << format("   avg={:8.2f} ms", averages[s][threadCount - 1]) << endl;
        }
        cout << endl;
    }

    cout << format("average time, ms ({} ops per thread)", opsPerThread) << endl;
    cout << format("{:12}{:>12}{:>12}{:>12}", "", "1 thread", "2 threads", "3 threads") << endl;
    for (size_t s = 0; s < scenarios.size(); ++s)
    {
        cout << format("{:<12}", scenarios[s].name);
        for (double avg : averages[s])
            cout << format("{:12.2f}", avg);
        cout << endl;
    }
    return 0;
}
