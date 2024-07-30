#include <catch2/generators/catch_generators.hpp> 
#include <concepts>
#include <utility>
#include <ansipp/integral.hpp>

template<std::unsigned_integral T>
class base_generator_impl final : public Catch::Generators::IGenerator<std::pair<unsigned int, T>> {
    unsigned int base;
    unsigned int max;
    std::pair<unsigned int, T> info;
public:
    base_generator_impl(unsigned int base, unsigned int min, unsigned int max): 
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
};

template <std::unsigned_integral T>
Catch::Generators::GeneratorWrapper<std::pair<unsigned int, T>> base_gen(
    unsigned int base, 
    unsigned int min = 0, 
    unsigned int max = std::numeric_limits<unsigned int>::max()
) {
    return Catch::Generators::GeneratorWrapper(new base_generator_impl<T>(base, min, max));
}
