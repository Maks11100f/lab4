#pragma once

#include <string>
#include <vector>

#include "DataStruct.h"
#include "Operation.h"

std::vector<Operation> loadOperations(const std::string &path);

double runInThreads(DataStruct &ds, const std::vector<std::vector<Operation>> &files, int threadCount);
