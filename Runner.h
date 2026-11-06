#pragma once

#include <string>
#include <vector>

#include "DataStruct.h"
#include "Operation.h"

std::vector<Operation> loadOperations(const std::string& path);

double runInThreads(DataStruct& ds, const std::vector<const std::vector<Operation>*>& perThread);
