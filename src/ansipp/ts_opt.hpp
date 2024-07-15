#pragma once

#include <concepts>
#include <atomic>

namespace ansipp {

template <typename T>
class ts_opt {
    T value = {};
    std::atomic_bool set = false;
public:
    bool is_set() const { return set.load(); }
    
    bool store(T&& v) {
        if (set.load()) return false;
        value = std::move(v);
        if (set.exchange(true) == true) return false;
        return true;
    }

    bool store(const T& v) { return store(T(v)); }

    template <std::regular_invocable<T> Callback>
    void restore(const Callback& v) { if (set.exchange(false)) v(value); }
};

}