#include "Runner.h"
#include "timing.h"

#include <format>
#include <fstream>
#include <latch>
#include <stdexcept>
#include <thread>

using namespace std;

vector<Operation> loadOperations(const string &path)
{
    ifstream in(path);
    if (!in)
        throw runtime_error(format("cannot open {}", path));

    vector<Operation> ops;
    string cmd;
    while (in >> cmd)
    {
        Operation op{OpType::String, 0, 0};
        if (cmd == "read")
        {
            op.type = OpType::Read;
            in >> op.field;
        }
        else if (cmd == "write")
        {
            op.type = OpType::Write;
            in >> op.field >> op.value;
        }
        else if (cmd != "string")
            throw runtime_error(format("unknown command '{}' in {}", cmd, path));

        if (!in || op.field < 0 || op.field >= DataStruct::M)
            throw runtime_error(format("bad line '{}' in {}", cmd, path));
        ops.push_back(op);
    }
    return ops;
}

static size_t execute(DataStruct &ds, const vector<Operation> &ops)
{
    size_t checksum = 0;
    for (const auto &op : ops)
    {
        if (op.type == OpType::Read)
            checksum += ds.get(op.field);
        else if (op.type == OpType::Write)
            ds.set(op.field, op.value);
        else
            checksum += string(ds).size();
    }
    return checksum;
}

double runInThreads(DataStruct &ds, const vector<vector<Operation>> &files, int threadCount)
{
    latch start(1);
    vector<size_t> checksums(threadCount);
    vector<jthread> threads;
    for (int t = 0; t < threadCount; ++t)
        threads.emplace_back([&, t]
                             {
                                 start.wait();
                                 checksums[t] = execute(ds, files[t]);
                             });

    return timeit([&]
                  {
                      start.count_down();
                      for (auto &th : threads)
                          th.join();
                  });
}
