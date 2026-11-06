#pragma once

#include <chrono>

template <class F>
double timeit(F &&f)
{
    auto time1 = std::chrono::steady_clock::now();
    f();
    auto time2 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(time2 - time1).count();
}
