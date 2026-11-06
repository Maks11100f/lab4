#pragma once

#include <string>
#include <vector>

#include "Operation.h"

struct OpShare {
    OpType type;
    int field;
    int percent;
};

struct Scenario {
    std::string name;
    std::vector<OpShare> shares;
};

std::vector<Scenario> defaultScenarios();

void generateFile(const std::string& path, const Scenario& scenario, int opCount, unsigned seed);
