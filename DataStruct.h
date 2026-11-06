#pragma once

#include <mutex>
#include <shared_mutex>
#include <string>

class DataStruct {
public:
    static constexpr int M = 2;

    int get(int i) const {
        std::shared_lock<std::shared_mutex> lock(fields_[i].mtx);
        return fields_[i].value;
    }

    void set(int i, int value) {
        std::unique_lock<std::shared_mutex> lock(fields_[i].mtx);
        fields_[i].value = value;
    }

    explicit operator std::string() const {
        std::shared_lock<std::shared_mutex> lock0(fields_[0].mtx, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock1(fields_[1].mtx, std::defer_lock);
        std::lock(lock0, lock1);
        const int value0 = fields_[0].value;
        const int value1 = fields_[1].value;
        lock0.unlock();
        lock1.unlock();
        return "[" + std::to_string(value0) + ", " + std::to_string(value1) + "]";
    }

private:
    struct alignas(64) Field {
        int value = 0;
        mutable std::shared_mutex mtx;
    };

    Field fields_[M];
};
