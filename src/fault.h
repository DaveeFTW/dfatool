#pragma once

#include "aestools.h"
#include "set.h"
#include "u128.h"

using Fault = u128;

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

struct FaultCandidateList
{
    std::array<FaultCandidate, 1024> candidates;
    std::size_t count{ 0 };

    bool solved() const
    {
        const auto& candidate = candidates[0];
        return candidate.values[0].count == 1 && candidate.values[1].count == 1 &&
               candidate.values[2].count == 1 && candidate.values[3].count == 1 && count == 1;
    }
};

void intersect_candidates(FaultCandidateList& candidates, const FaultCandidateList& new_candidates)
{
    FaultCandidateList intersect;

    for (auto i = 0u; i < new_candidates.count; ++i)
    {
        for (auto j = 0u; j < candidates.count; ++j)
        {
            if (candidates.candidates[j].intersects(new_candidates.candidates[i]))
            {
                intersect.candidates[intersect.count++] =
                    candidates.candidates[j].intersect(new_candidates.candidates[i]);
            }
        }
    }

    if (intersect.count != 0)
    {
        candidates = intersect;
    }
    else
    {
        for (auto i = 0u; i < new_candidates.count; ++i)
        {
            candidates.candidates[candidates.count++] = new_candidates.candidates[i];
        }
    }
}
