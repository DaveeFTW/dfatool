#pragma once

#include "aestools.h"
#include "set.h"

struct FaultCandidate
{
    void add(std::uint8_t value, int index)
    {
        auto& v = values[index];
        v.values[v.count++] = value;
    }

    bool intersects(const FaultCandidate& other) const
    {
        return values[0].intersects(other.values[0]) && values[1].intersects(other.values[1]) &&
               values[2].intersects(other.values[2]) && values[3].intersects(other.values[3]);
    }

    FaultCandidate intersect(const FaultCandidate& other) const
    {
        FaultCandidate intersect_candidate;
        intersect_candidate.values[0] = values[0].intersect(other.values[0]);
        intersect_candidate.values[1] = values[1].intersect(other.values[1]);
        intersect_candidate.values[2] = values[2].intersect(other.values[2]);
        intersect_candidate.values[3] = values[3].intersect(other.values[3]);
        return intersect_candidate;
    }

    std::array<Set, 4> values;
};
