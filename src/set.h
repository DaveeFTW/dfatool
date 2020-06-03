#pragma once

#include <array>

struct Set
{
    Set intersect(const Set& other) const
    {
        Set intersect_values;

        for (auto i = 0u; i < count; ++i)
        {
            for (auto j = 0u; j < other.count; ++j)
            {
                if (other.values[j] == values[i])
                {
                    intersect_values.values[intersect_values.count++] = values[i];
                }
            }
        }

        return intersect_values;
    }

    bool intersects(const Set& other) const
    {
        for (auto i = 0u; i < count; ++i)
        {
            for (auto j = 0u; j < other.count; ++j)
            {
                if (other.values[j] == values[i])
                {
                    return true;
                }
            }
        }

        return false;
    }

    std::array<std::uint8_t, 4> values;
    std::size_t count{ 0u };
};
