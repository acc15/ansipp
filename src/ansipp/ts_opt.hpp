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
        // there is possible race condition when 2 (or more) parallel init are running
        // check below makes occur it less frequently, but doesn't solve it completely.
        if (set.load()) return false;
        
        // when both threads set this value
        value = std::move(v); 

        // but 2nd will fail here, value may contain illegal data (from 2nd thread)
        if (set.exchange(true) == true) return false;

        // generally init() shouldn't be used from multiple threads, so it's not a problem for now
        return true;
    }

    bool store(const T& v) { return store(T(v)); }

    template <std::regular_invocable<T> Callback>
    void restore(const Callback& v) { if (set.exchange(false)) v(static_cast<const T&>(value)); }
};

}