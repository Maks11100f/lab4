#include "Generator.h"

#include <algorithm>
#include <format>
#include <fstream>
#include <random>
#include <stdexcept>

using namespace std;

vector<Scenario> defaultScenarios()
{
    return {
        {"a_variant", {
                          {OpType::Read, 0, 10},
                          {OpType::Write, 0, 5},
                          {OpType::Read, 1, 50},
                          {OpType::Write, 1, 5},
                          {OpType::String, 0, 30},
                      }},
        {"b_equal", {
                        {OpType::Read, 0, 20},
                        {OpType::Write, 0, 20},
                        {OpType::Read, 1, 20},
                        {OpType::Write, 1, 20},
                        {OpType::String, 0, 20},
                    }},
        {"c_skewed", {
                         {OpType::Read, 0, 2},
                         {OpType::Write, 0, 70},
                         {OpType::Read, 1, 3},
                         {OpType::Write, 1, 20},
                         {OpType::String, 0, 5},
                     }},
    };
}

void generateFile(const string &path, const Scenario &scenario, int opCount, unsigned seed)
{
    mt19937 e(seed);
    uniform_int_distribution<int> d(0, 999);

    vector<Operation> ops;
    for (size_t s = 0; s < scenario.shares.size(); ++s)
    {
        const auto &share = scenario.shares[s];
        long long count = static_cast<long long>(opCount) * share.percent / 100;
        if (s + 1 == scenario.shares.size())
            count = opCount - static_cast<long long>(ops.size());
        for (long long k = 0; k < count; ++k)
            ops.push_back({share.type, share.field, d(e)});
    }
    shuffle(ops.begin(), ops.end(), e);

    ofstream out(path);
    if (!out)
        throw runtime_error(format("cannot open {} for writing", path));
    for (const auto &op : ops)
    {
        if (op.type == OpType::Read)
            out << format("read {}\n", op.field);
        else if (op.type == OpType::Write)
            out << format("write {} {}\n", op.field, op.value);
        else
            out << "string\n";
    }
}
