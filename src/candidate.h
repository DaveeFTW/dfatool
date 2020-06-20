#pragma once

#include "aestools.h"
#include "set.h"
#include "u128.h"

#include <array>
#include <vector>

struct FaultCandidate
{
    void add(std::uint8_t value, int index);
    bool intersects(const FaultCandidate& other) const;
    FaultCandidate intersect(const FaultCandidate& other) const;

    std::array<Set, 4> values;
};

struct FaultCandidateList
{
    bool solved() const;

    std::vector<FaultCandidate> candidates;
};

void intersect_candidates(FaultCandidateList& candidates, const FaultCandidateList& new_candidates);
