#pragma once

#include <concepts>
#include <atomic>

namespace ansipp {

template <typename T>
struct ts_opt {
    std::atomic<T> value;
    std::atomic_bool is_set = false;

    static_assert(std::atomic<T>::is_always_lock_free, "atomic type is not lock-free");

    void store(const T& v) {
        if ( is_set.exchange(true) ) {
            throw std::logic_error(
                "double initialization will cause very odd bugs, "
                "call restore() before calling init()");
        }
        value.store(v);
    }

    template <std::regular_invocable<T> Callback>
    void restore(const Callback& v) {
        if ( is_set.exchange(false) ) {
            v(value.load());
        }
    }

};

}