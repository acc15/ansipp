#include <catch2/generators/catch_generators.hpp> 
#include <concepts>
#include <utility>
#include <limits>
#include <ansipp/integral.hpp>

template<std::unsigned_integral T>
class pow_gen final : public Catch::Generators::IGenerator<std::pair<unsigned int, T>> {
    unsigned int base;
    unsigned int max;
    std::pair<unsigned int, T> info;
public:
    pow_gen(unsigned int base, unsigned int min, unsigned int max): 
        base(base), max(max), info(min + 1, ansipp::cpow<T>(base, min)) {}

    std::pair<unsigned int, T> const& get() const override { return info; }
    bool next() override {
        if (info.first > max) return false;
        T new_value = info.second * base;
        if (new_value / base != info.second) return false; // overflow
        ++info.first;
        info.second = new_value;
        return true;
    }

    static Catch::Generators::GeneratorWrapper<std::pair<unsigned int, T>> wrap(
        unsigned int base, 
        unsigned int min = 0, 
        unsigned int max = std::numeric_limits<unsigned int>::max()
    ) {
        return Catch::Generators::GeneratorWrapper(new pow_gen(base, min, max));
    }
};
