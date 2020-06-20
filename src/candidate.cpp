#include "candidate.h"

void FaultCandidate::add(std::uint8_t value, int index)
{
    auto& v = values[index];
    v.values[v.count++] = value;
}

bool FaultCandidate::intersects(const FaultCandidate& other) const
{
    return values[0].intersects(other.values[0]) && values[1].intersects(other.values[1]) &&
           values[2].intersects(other.values[2]) && values[3].intersects(other.values[3]);
}

FaultCandidate FaultCandidate::intersect(const FaultCandidate& other) const
{
    FaultCandidate intersect_candidate;
    intersect_candidate.values[0] = values[0].intersect(other.values[0]);
    intersect_candidate.values[1] = values[1].intersect(other.values[1]);
    intersect_candidate.values[2] = values[2].intersect(other.values[2]);
    intersect_candidate.values[3] = values[3].intersect(other.values[3]);
    return intersect_candidate;
}

bool FaultCandidateList::solved() const
{
    if (candidates.size() != 1)
    {
        return false;
    }

    const auto& candidate = candidates[0];
    return candidate.values[0].count == 1 && candidate.values[1].count == 1 &&
           candidate.values[2].count == 1 && candidate.values[3].count == 1;
}

void intersect_candidates(FaultCandidateList& candidates, const FaultCandidateList& new_candidates)
{
    FaultCandidateList intersect;

    for (const auto& new_candidate : new_candidates.candidates)
    {
        for (const auto& candidate : candidates.candidates)
        {
            if (candidate.intersects(new_candidate))
            {
                intersect.candidates.push_back(candidate.intersect(new_candidate));
            }
        }
    }

    if (!intersect.candidates.empty())
    {
        candidates = intersect;
    }
    else
    {
        for (const auto& new_candidate : new_candidates.candidates)
        {
            candidates.candidates.push_back(new_candidate);
        }
    }
}
