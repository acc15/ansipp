#pragma once

#include <concepts>
#include <atomic>
#include <cstdlib>

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

        // but 2nd will fail here, and value may hold corrupted data, 
        // the best decision is to abort early than catch very strange bugs
        if (set.exchange(true) == true) std::abort();

        // generally init() shouldn't be used from multiple threads, so it's not a problem for now
        return true;
    }

    bool store(const T& v) { return store(T(v)); }

    template <typename Callback>
    void restore(const Callback& v) { if (set.exchange(false)) v(value); }
};

}