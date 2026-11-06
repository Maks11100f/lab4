#include "Generator.h"

#include <algorithm>
#include <fstream>
#include <random>
#include <stdexcept>

std::vector<Scenario> defaultScenarios() {
    return {
        {"a_variant", {
            {OpType::Read,   0, 10},
            {OpType::Write,  0, 5},
            {OpType::Read,   1, 50},
            {OpType::Write,  1, 5},
            {OpType::String, 0, 30},
        }},
        {"b_equal", {
            {OpType::Read,   0, 20},
            {OpType::Write,  0, 20},
            {OpType::Read,   1, 20},
            {OpType::Write,  1, 20},
            {OpType::String, 0, 20},
        }},
        {"c_skewed", {
            {OpType::Read,   0, 2},
            {OpType::Write,  0, 70},
            {OpType::Read,   1, 3},
            {OpType::Write,  1, 20},
            {OpType::String, 0, 5},
        }},
    };
}

static std::vector<Operation> buildOperations(const Scenario& scenario, int opCount, std::mt19937& rng) {
    std::uniform_int_distribution<int> valueDist(0, 999);
    std::vector<Operation> ops;
    ops.reserve(opCount);

    int percentSum = 0;
    for (const OpShare& share : scenario.shares) {
        percentSum += share.percent;
    }
    if (percentSum != 100) {
        throw std::invalid_argument("percentages of scenario " + scenario.name + " do not sum to 100");
    }

    for (std::size_t s = 0; s < scenario.shares.size(); ++s) {
        const OpShare& share = scenario.shares[s];
        long long count = static_cast<long long>(opCount) * share.percent / 100;
        if (s + 1 == scenario.shares.size()) {
            count = opCount - static_cast<long long>(ops.size());
        }
        for (long long k = 0; k < count; ++k) {
            ops.push_back({share.type, share.field, valueDist(rng)});
        }
    }

    std::shuffle(ops.begin(), ops.end(), rng);
    return ops;
}

void generateFile(const std::string& path, const Scenario& scenario, int opCount, unsigned seed) {
    std::mt19937 rng(seed);
    std::vector<Operation> ops = buildOperations(scenario, opCount, rng);

    std::string text;
    text.reserve(static_cast<std::size_t>(opCount) * 10);
    for (const Operation& op : ops) {
        switch (op.type) {
            case OpType::Read:
                text += "read " + std::to_string(op.field) + '\n';
                break;
            case OpType::Write:
                text += "write " + std::to_string(op.field) + ' ' + std::to_string(op.value) + '\n';
                break;
            case OpType::String:
                text += "string\n";
                break;
        }
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("cannot open " + path + " for writing");
    }
    out << text;
}
