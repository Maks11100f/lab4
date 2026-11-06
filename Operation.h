#pragma once

enum class OpType { Read, Write, String };

struct Operation {
    OpType type;
    int field;
    int value;
};
