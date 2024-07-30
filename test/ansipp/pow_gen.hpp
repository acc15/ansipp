#include <catch2/generators/catch_generators.hpp> 
#include <concepts>
#include <utility>
#include <limits>
#include <algorithm>
#include <ansipp/integral.hpp>

template<std::unsigned_integral T>
class pow_gen final : public Catch::Generators::IGenerator<std::pair<unsigned int, T>> {
    
    unsigned int base;
    unsigned int max_digits;
    std::pair<unsigned int, T> info;
    
    pow_gen(unsigned int base, unsigned int min_digits, unsigned int max_digits): 
        base(base), 
        max_digits(max_digits), 
        info(min_digits, ansipp::ipow<T>(base, min_digits - 1)) 
    {}

public:
    std::pair<unsigned int, T> const& get() const override {
         return info; 
    }
    
    bool next() override {
        if (info.first >= max_digits) return false;
        T new_value = info.second * base;
        if (new_value / base != info.second) return false; // overflow
        ++info.first;
        info.second = new_value;
        return true;
    }

    static Catch::Generators::GeneratorWrapper<std::pair<unsigned int, T>> wrap(
        unsigned int base, 
        unsigned int min_digits = 1, 
        unsigned int max_digits = std::numeric_limits<unsigned int>::max()
    ) {
        unsigned int min = std::max(1U, min_digits);
        unsigned int max = std::max(min, max_digits);
        return Catch::Generators::GeneratorWrapper(new pow_gen(base, min, max));
    }
};
