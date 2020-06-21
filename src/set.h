#pragma once

#include <array>

struct Set
{
    Set intersect(const Set& other) const;
    bool intersects(const Set& other) const;

    std::array<std::uint8_t, 4> values;
    std::size_t count{ 0u };
};
