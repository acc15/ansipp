#pragma once

#include <concepts>
#include <atomic>

namespace ansipp {

template <typename T>
class ts_opt {
    T value = {};
    std::atomic_bool is_set = false;
public:
    bool store(T&& v) {
        if (is_set.load()) { return false; }
        value = std::move(v);
        if (is_set.exchange(true) == true) { return false; }
        return true;
    }

    bool store(const T& v) {
        return store(T(v));
    }

    template <std::regular_invocable<T> Callback>
    void restore(const Callback& v) {
        if (is_set.exchange(false)) { v(value); }
    }
};

}