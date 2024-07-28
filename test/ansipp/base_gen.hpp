#include <catch2/generators/catch_generators.hpp> 
#include <concepts>
#include <ansipp/integral.hpp>

template <typename T>
struct base_info {
    using type = T;
    unsigned int pow;
    type value;
};

template<std::unsigned_integral T>
class base_generator_impl final : public Catch::Generators::IGenerator<base_info<T>> {
    unsigned int base;
    unsigned int max;
    base_info<T> info;
public:
    base_generator_impl(unsigned int base, unsigned int min, unsigned int max): 
        base(base), max(max), info { min, ansipp::cpow<T>(base, min) } {}

    base_info<T> const& get() const override { return info; }
    bool next() override {
        if (info.pow >= max) return false;
        T new_value = info.value * base;
        if (new_value / base != info.value) return false; // overflow
        ++info.pow;
        info.value = new_value;
        return true;
    }
};

template <std::unsigned_integral T>
Catch::Generators::GeneratorWrapper<base_info<T>> base_gen(
    unsigned int base, 
    unsigned int min = 0, 
    unsigned int max = std::numeric_limits<unsigned int>::max()
) {
    return Catch::Generators::GeneratorWrapper<base_info<T>>(new base_generator_impl<T>(base, min, max));
}
