#pragma once

#include <mutex>
#include <shared_mutex>
#include <string>

class DataStruct
{
public:
    static constexpr int M = 2;

    DataStruct() = default;
    DataStruct(const DataStruct &) = delete;
    DataStruct &operator=(const DataStruct &) = delete;
    ~DataStruct() = default;

    int get(int i) const
    {
        std::shared_lock lock(fields[i].mtx);
        return fields[i].value;
    }

    void set(int i, int value)
    {
        std::lock_guard lock(fields[i].mtx);
        fields[i].value = value;
    }

    explicit operator std::string() const
    {
        int value0, value1;
        {
            std::shared_lock lock0(fields[0].mtx, std::defer_lock);
            std::shared_lock lock1(fields[1].mtx, std::defer_lock);
            std::lock(lock0, lock1);
            value0 = fields[0].value;
            value1 = fields[1].value;
        }
        return "[" + std::to_string(value0) + ", " + std::to_string(value1) + "]";
    }

private:
    struct alignas(64) Field
    {
        int value = 0;
        mutable std::shared_mutex mtx;
    };

    Field fields[M];
};
