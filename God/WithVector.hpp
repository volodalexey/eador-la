#pragma once

#include <vector>

template<class T>
class WithVector {
public:
    std::vector<T> items;
    explicit WithVector(std::vector<T> items) : items(std::move(items)) {}
    WithVector(std::initializer_list<T> items) : WithVector(std::vector(items)) {}
    template <std::convertible_to<T>... Args>
    explicit WithVector(Args... args) : WithVector({ args... }) {}
    template<std::ranges::input_range Range>
    explicit WithVector(Range regions) : WithVector(std::vector(regions.begin(), regions.end())) {}
};
