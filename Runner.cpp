#include "Runner.h"

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <thread>

std::vector<Operation> loadOperations(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("cannot open " + path);
    }

    std::vector<Operation> ops;
    std::string cmd;
    while (in >> cmd) {
        Operation op{OpType::String, 0, 0};
        if (cmd == "read") {
            op.type = OpType::Read;
            in >> op.field;
        } else if (cmd == "write") {
            op.type = OpType::Write;
            in >> op.field >> op.value;
        } else if (cmd != "string") {
            throw std::runtime_error("unknown command '" + cmd + "' in " + path);
        }
        if (!in || op.field < 0 || op.field >= DataStruct::M) {
            throw std::runtime_error("bad line in " + path + " after command '" + cmd + "'");
        }
        ops.push_back(op);
    }
    return ops;
}

static std::size_t execute(DataStruct& ds, const std::vector<Operation>& ops) {
    std::size_t sink = 0;
    for (const Operation& op : ops) {
        switch (op.type) {
            case OpType::Read:
                sink += ds.get(op.field);
                break;
            case OpType::Write:
                ds.set(op.field, op.value);
                break;
            case OpType::String:
                sink += std::string(ds).size();
                break;
        }
    }
    return sink;
}

double runInThreads(DataStruct& ds, const std::vector<const std::vector<Operation>*>& perThread) {
    const std::size_t threadCount = perThread.size();

    std::mutex gateMutex;
    std::condition_variable gateCv;
    std::size_t ready = 0;
    bool go = false;

    std::vector<std::size_t> sinks(threadCount, 0);
    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (std::size_t t = 0; t < threadCount; ++t) {
        threads.emplace_back([&, t] {
            {
                std::unique_lock<std::mutex> lock(gateMutex);
                ++ready;
                gateCv.notify_all();
                gateCv.wait(lock, [&] { return go; });
            }
            sinks[t] = execute(ds, *perThread[t]);
        });
    }

    {
        std::unique_lock<std::mutex> lock(gateMutex);
        gateCv.wait(lock, [&] { return ready == threadCount; });
    }

    const auto start = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(gateMutex);
        go = true;
    }
    gateCv.notify_all();

    for (std::thread& th : threads) {
        th.join();
    }
    const auto finish = std::chrono::steady_clock::now();

    volatile std::size_t keep = 0;
    for (std::size_t s : sinks) {
        keep = keep + s;
    }

    return std::chrono::duration<double, std::milli>(finish - start).count();
}
