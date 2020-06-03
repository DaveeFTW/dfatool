#pragma once

#include "aestools.h"
#include "fault.h"
#include "u128.h"

#include <array>
#include <cstdint>
#include <vector>

using Fault = u128;

struct EncryptDFA
{
    static constexpr auto blend_mask()
    {
        constexpr auto F = -1;
        constexpr auto O = 0;

        // clang-format off
        constexpr std::array mask0 = {F, O, O, O,
                                      O, O, O, F,
                                      O, O, F, O,
                                      O, F, O, O};

        constexpr std::array mask1 = {O, F, O, O,
                                      F, O, O, O,
                                      O, O, O, F,
                                      O, O, F, O};

        constexpr std::array mask2 = {O, O, F, O,
                                      O, F, O, O,
                                      F, O, O, O,
                                      O, O, O, F};

        constexpr std::array mask3 = {O, O, O, F,
                                      O, O, F, O,
                                      O, F, O, O,
                                      F, O, O, O};

        // clang-format on

        return std::array{ mask0, mask1, mask2, mask3 };
    }

    static std::array<std::uint8_t, 256> inverse_box;
    static std::array<std::uint8_t, 256> forward_box;

    static constexpr int FaultIndex[4][4] = { { 0, 7, 10, 13 },
                                              { 1, 4, 11, 14 },
                                              { 2, 5, 8, 15 },
                                              { 3, 6, 9, 12 } };

    static constexpr std::uint8_t mix_columns_matrix[4][4] = { { 2, 3, 1, 1 },
                                                               { 1, 2, 3, 1 },
                                                               { 1, 1, 2, 3 },
                                                               { 3, 1, 1, 2 } };
};

std::array<std::uint8_t, 256> EncryptDFA::inverse_box = { aes_isbox };
std::array<std::uint8_t, 256> EncryptDFA::forward_box = { aes_sbox };

struct DecryptDFA
{
    static constexpr auto blend_mask()
    {
        constexpr auto F = true;
        constexpr auto O = false;

        // clang-format off
        constexpr std::array mask0 = {F, O, O, O,
                                      O, F, O, O,
                                      O, O, F, O,
                                      O, O, O, F};

        constexpr std::array mask1 = {O, F, O, O,
                                      O, O, F, O,
                                      O, O, O, F,
                                      F, O, O, O};

        constexpr std::array mask2 = {O, O, F, O,
                                      O, O, O, F,
                                      F, O, O, O,
                                      O, F, O, O};

        constexpr std::array mask3 = {O, O, O, F,
                                      F, O, O, O,
                                      O, F, O, O,
                                      O, O, F, O};

        // clang-format on

        return std::array{ mask0, mask1, mask2, mask3 };
    }

    static std::array<std::uint8_t, 256> inverse_box;
    static std::array<std::uint8_t, 256> forward_box;

    static constexpr int FaultIndex[4][4] = { { 0, 5, 10, 15 },
                                              { 3, 4, 9, 14 },
                                              { 2, 7, 8, 13 },
                                              { 1, 6, 11, 12 } };

    static constexpr std::uint8_t mix_columns_matrix[4][4] = { { 14, 9, 13, 11 },
                                                               { 11, 14, 9, 13 },
                                                               { 13, 11, 14, 9 },
                                                               { 9, 13, 11, 14 } };
};

std::array<std::uint8_t, 256> DecryptDFA::inverse_box = { aes_sbox };
std::array<std::uint8_t, 256> DecryptDFA::forward_box = { aes_isbox };

template<typename DFA>
static inline std::array<Fault, 4> convert_r8_fault(Fault r8_fault, Fault r9_reference)
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

template<typename DFA>
static inline std::array<std::uint8_t, 256> generate_zmap(std::uint8_t diff,
                                                          std::uint8_t multiplier)
{
    // we have the equation:
    // Y0 = S(X0) + S(C*Z + X0)
    // where Y0 is our diff, C is our multiplier, Z is our fault, and X0 is some unknown AES state
    // we can rework this:
    // Y0 = S(X0) + S(C*Z + X0)
    // S(C*Z + X0) = Y0 + S(X0)
    // C*Z + X0 = InvSBox(Y0 + S(X0))
    // C*Z = InvSBox(Y0 + S(X0)) + X0
    // Z = InvMult(C)*(InvSBox(Y0 + S(X0)) + X0)
    std::array<std::uint8_t, 256> zmap;

    for (auto X0 = 0u; X0 < zmap.size(); ++X0)
    {
        zmap[X0] = aes_gf_imult[multiplier][DFA::inverse_box[diff ^ DFA::forward_box[X0]] ^ X0];
    }

    return zmap;
}

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

template<typename DFA, int group, int faultrow>
void calculate_fault_candidates_for_fault(FaultCandidateList& candidates, Fault fault, Fault ref)
{
    constexpr auto Row0Intersection = (1 << 0);
    constexpr auto Row1Intersection = (1 << 1);
    constexpr auto Row2Intersection = (1 << 2);
    constexpr auto Row3Intersection = (1 << 3);
    constexpr auto AllRowIntersection =
        Row0Intersection | Row1Intersection | Row2Intersection | Row3Intersection;

    const auto diff = xor128(fault, ref);
    const auto row0 =
        generate_zmap<DFA>(diff[DFA::FaultIndex[group][0]], DFA::mix_columns_matrix[faultrow][0]);
    const auto row1 =
        generate_zmap<DFA>(diff[DFA::FaultIndex[group][1]], DFA::mix_columns_matrix[faultrow][1]);
    const auto row2 =
        generate_zmap<DFA>(diff[DFA::FaultIndex[group][2]], DFA::mix_columns_matrix[faultrow][2]);
    const auto row3 =
        generate_zmap<DFA>(diff[DFA::FaultIndex[group][3]], DFA::mix_columns_matrix[faultrow][3]);

    std::array<std::uint8_t, 256> intersections = { 0 };

    for (auto i = 0; i < 256; ++i)
    {
        intersections[row0[i]] |= Row0Intersection;
        intersections[row1[i]] |= Row1Intersection;
        intersections[row2[i]] |= Row2Intersection;
        intersections[row3[i]] |= Row3Intersection;
    }

    for (auto i = 0u; i < intersections.size(); ++i)
    {
        // we only care about intersections that appear in each row
        if (intersections[i] != AllRowIntersection)
            continue;

        auto& candidate = candidates.candidates[candidates.count++];

        for (auto j = 0u; j < 256u; ++j)
        {
            if (row0[j] == i)
            {
                candidate.add(DFA::forward_box[j] ^ ref[DFA::FaultIndex[group][0]], 0);
            }

            if (row1[j] == i)
            {
                candidate.add(DFA::forward_box[j] ^ ref[DFA::FaultIndex[group][1]], 1);
            }

            if (row2[j] == i)
            {
                candidate.add(DFA::forward_box[j] ^ ref[DFA::FaultIndex[group][2]], 2);
            }

            if (row3[j] == i)
            {
                candidate.add(DFA::forward_box[j] ^ ref[DFA::FaultIndex[group][3]], 3);
            }
        }
    }
}

template<typename DFA, int group>
static inline FaultCandidateList calculate_fault_candidate(Fault fault, Fault ref)
{
    FaultCandidateList candidates;
    calculate_fault_candidates_for_fault<DFA, group, 0>(candidates, fault, ref);
    calculate_fault_candidates_for_fault<DFA, group, 1>(candidates, fault, ref);
    calculate_fault_candidates_for_fault<DFA, group, 2>(candidates, fault, ref);
    calculate_fault_candidates_for_fault<DFA, group, 3>(candidates, fault, ref);
    return candidates;
}

template<typename DFA, int group>
static inline bool is_group_affected(const Fault fault, const Fault ref)
{
    const auto diff = xor128(fault, ref);
    const auto row0diff = diff[DFA::FaultIndex[group][0]];
    const auto row1diff = diff[DFA::FaultIndex[group][1]];
    const auto row2diff = diff[DFA::FaultIndex[group][2]];
    const auto row3diff = diff[DFA::FaultIndex[group][3]];
    const auto affected = row0diff != 0 && row1diff != 0 && row2diff != 0 && row3diff != 0;
    return affected;
}

static inline void intersect_candidates(FaultCandidateList& candidates,
                                        const FaultCandidateList& new_candidates)
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

template<typename DFA, typename T>
static inline std::array<FaultCandidateList, 4> solve_r9_candidates(const T& r9_faults,
                                                                    const Fault ref) noexcept
{
    std::array<FaultCandidateList, 4> keyCandidates;

    for (const auto& fault : r9_faults)
    {
        if (is_group_affected<DFA, 0>(fault, ref) && !keyCandidates[0].solved())
        {
            const auto newCandidates = calculate_fault_candidate<DFA, 0>(fault, ref);
            intersect_candidates(keyCandidates[0], newCandidates);
        }

        else if (is_group_affected<DFA, 1>(fault, ref) && !keyCandidates[1].solved())
        {
            const auto newCandidates = calculate_fault_candidate<DFA, 1>(fault, ref);
            intersect_candidates(keyCandidates[1], newCandidates);
        }

        else if (is_group_affected<DFA, 2>(fault, ref) && !keyCandidates[2].solved())
        {
            const auto newCandidates = calculate_fault_candidate<DFA, 2>(fault, ref);
            intersect_candidates(keyCandidates[2], newCandidates);
        }

        else if (is_group_affected<DFA, 3>(fault, ref) && !keyCandidates[3].solved())
        {
            const auto newCandidates = calculate_fault_candidate<DFA, 3>(fault, ref);
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
