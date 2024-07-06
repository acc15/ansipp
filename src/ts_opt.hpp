#pragma once

#include <concepts>
#include <atomic>

namespace ansipp {

template <typename T>
class ts_opt {
    T value = {};
    std::atomic_bool is_set = false;

    inline std::logic_error is_set_condition_failed() {
        return std::logic_error(
            "double initialization will cause very odd bugs, "
            "call restore() before calling init()");
    }

public:

    void store(const T& v) {
        if ( is_set.load() ) {
            throw is_set_condition_failed();
        }
        value = v;
        if ( is_set.exchange(true) == true ) {
            throw is_set_condition_failed();
        }
    }

    template <std::regular_invocable<T> Callback>
    void restore(const Callback& v) {
        if ( is_set.exchange(false) ) {
            v(value);
        }
    }

};

}