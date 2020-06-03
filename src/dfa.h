#pragma once

#include "aestools.h"
#include "differentialfault.h"
#include "fault.h"

#include <array>
#include <cstdint>
#include <vector>

template<typename DFA>
static inline auto convert_r8_fault(Fault r8_fault, Fault r9_reference)
{
    const auto masks = DFA::blend_mask();
    static_assert(masks.size() == 4);

    std::array<Fault, 4> r9_faults;

    for (auto i = 0u; i < sizeof(Fault); ++i)
    {
        r9_faults[0][i] = masks[0][i] ? (r8_fault[i]) : (r9_reference[i]);
        r9_faults[1][i] = masks[1][i] ? (r8_fault[i]) : (r9_reference[i]);
        r9_faults[2][i] = masks[2][i] ? (r8_fault[i]) : (r9_reference[i]);
        r9_faults[3][i] = masks[3][i] ? (r8_fault[i]) : (r9_reference[i]);
    }

    return r9_faults;
}

template<typename DFA, typename T>
static inline std::array<FaultCandidateList, 4> solve_r9_candidates(const T& r9_faults,
                                                                    const Fault ref) noexcept
{
    std::array<FaultCandidateList, 4> keyCandidates;

    for (const auto& r9_fault : r9_faults)
    {
        DifferentialFault<DFA> fault{ r9_fault, ref };

        if (fault.is_group_affected(0) && !keyCandidates[0].solved())
        {
            const auto newCandidates = fault.candidates_for_group(0);
            intersect_candidates(keyCandidates[0], newCandidates);
        }

        else if (fault.is_group_affected(1) && !keyCandidates[1].solved())
        {
            const auto newCandidates = fault.candidates_for_group(1);
            intersect_candidates(keyCandidates[1], newCandidates);
        }

        else if (fault.is_group_affected(2) && !keyCandidates[2].solved())
        {
            const auto newCandidates = fault.candidates_for_group(2);
            intersect_candidates(keyCandidates[2], newCandidates);
        }

        else if (fault.is_group_affected(3) && !keyCandidates[3].solved())
        {
            const auto newCandidates = fault.candidates_for_group(3);
            intersect_candidates(keyCandidates[3], newCandidates);
        }
    }

    return keyCandidates;
}

template<typename DFA>
bool solve_r9(const std::vector<Fault>& r9_faults, const Fault ref) noexcept
{
    const auto groupIntersections = solve_r9_candidates<DFA>(r9_faults, ref);

    if (groupIntersections[0].solved() && groupIntersections[1].solved() &&
        groupIntersections[2].solved() && groupIntersections[3].solved())
    {
        std::array<uint8_t, 16> data;

        data[DFA::FaultIndex[0][0]] = groupIntersections[0].candidates[0].values[0].values[0];
        data[DFA::FaultIndex[0][1]] = groupIntersections[0].candidates[0].values[1].values[0];
        data[DFA::FaultIndex[0][2]] = groupIntersections[0].candidates[0].values[2].values[0];
        data[DFA::FaultIndex[0][3]] = groupIntersections[0].candidates[0].values[3].values[0];

        data[DFA::FaultIndex[1][0]] = groupIntersections[1].candidates[0].values[0].values[0];
        data[DFA::FaultIndex[1][1]] = groupIntersections[1].candidates[0].values[1].values[0];
        data[DFA::FaultIndex[1][2]] = groupIntersections[1].candidates[0].values[2].values[0];
        data[DFA::FaultIndex[1][3]] = groupIntersections[1].candidates[0].values[3].values[0];

        data[DFA::FaultIndex[2][0]] = groupIntersections[2].candidates[0].values[0].values[0];
        data[DFA::FaultIndex[2][1]] = groupIntersections[2].candidates[0].values[1].values[0];
        data[DFA::FaultIndex[2][2]] = groupIntersections[2].candidates[0].values[2].values[0];
        data[DFA::FaultIndex[2][3]] = groupIntersections[2].candidates[0].values[3].values[0];

        data[DFA::FaultIndex[3][0]] = groupIntersections[3].candidates[0].values[0].values[0];
        data[DFA::FaultIndex[3][1]] = groupIntersections[3].candidates[0].values[1].values[0];
        data[DFA::FaultIndex[3][2]] = groupIntersections[3].candidates[0].values[2].values[0];
        data[DFA::FaultIndex[3][3]] = groupIntersections[3].candidates[0].values[3].values[0];

        printf("Found key: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
               data.data()[0],
               data.data()[1],
               data.data()[2],
               data.data()[3],
               data.data()[4],
               data.data()[5],
               data.data()[6],
               data.data()[7],
               data.data()[8],
               data.data()[9],
               data.data()[10],
               data.data()[11],
               data.data()[12],
               data.data()[13],
               data.data()[14],
               data.data()[15]);

        return true;
    }

    return false;
}

template<typename DFA>
bool solve_r8(const std::vector<Fault>& r8_faults, const Fault ref) noexcept
{
    std::vector<Fault> r9_faults;
    r9_faults.reserve(r8_faults.size() * 4);

    for (auto& fault : r8_faults)
    {
        auto converted = convert_r8_fault<DFA>(fault, ref);
        r9_faults.insert(r9_faults.end(), converted.begin(), converted.end());
    }

    return solve_r9<DFA>(r9_faults, ref);
}
