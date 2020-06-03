#pragma once

#include "fault.h"

template<typename DFA>
class DifferentialFault
{
public:
    DifferentialFault(const Fault& fault, const Fault& reference)
        : m_fault{ fault }
        , m_ref{ reference }
        , m_diff{ xor128(m_fault, m_ref) }
    {
    }

    bool is_group_affected(int group)
    {
        const auto row0diff = m_diff[DFA::FaultIndex[group][0]];
        const auto row1diff = m_diff[DFA::FaultIndex[group][1]];
        const auto row2diff = m_diff[DFA::FaultIndex[group][2]];
        const auto row3diff = m_diff[DFA::FaultIndex[group][3]];
        const auto affected = row0diff != 0 && row1diff != 0 && row2diff != 0 && row3diff != 0;
        return affected;
    }

    FaultCandidateList candidates_for_group(int group)
    {
        FaultCandidateList candidates;
        candidates_for_fault(candidates, group, 0);
        candidates_for_fault(candidates, group, 1);
        candidates_for_fault(candidates, group, 2);
        candidates_for_fault(candidates, group, 3);
        return candidates;
    }

private:
    auto generate_zmap(std::uint8_t diff, std::uint8_t multiplier)
    {
        // we have the equation:
        // Y0 = S(X0) + S(C*Z + X0)
        // where Y0 is our diff, C is our multiplier, Z is our fault, and X0 is some unknown AES
        // state we can rework this: Y0 = S(X0) + S(C*Z + X0) S(C*Z + X0) = Y0 + S(X0) C*Z + X0 =
        // InvSBox(Y0 + S(X0)) C*Z = InvSBox(Y0 + S(X0)) + X0 Z = InvMult(C)*(InvSBox(Y0 + S(X0)) +
        // X0)
        std::array<std::uint8_t, 256> zmap;

        for (auto X0 = 0u; X0 < zmap.size(); ++X0)
        {
            zmap[X0] = aes_gf_imult[multiplier][DFA::inverse_box[diff ^ DFA::forward_box[X0]] ^ X0];
        }

        return zmap;
    }

    void candidates_for_fault(FaultCandidateList& candidates, int group, int faultrow)
    {
        constexpr auto Row0Intersection = (1 << 0);
        constexpr auto Row1Intersection = (1 << 1);
        constexpr auto Row2Intersection = (1 << 2);
        constexpr auto Row3Intersection = (1 << 3);
        constexpr auto AllRowIntersection =
            Row0Intersection | Row1Intersection | Row2Intersection | Row3Intersection;

        const auto row0 =
            generate_zmap(m_diff[DFA::FaultIndex[group][0]], DFA::mix_columns_matrix[faultrow][0]);
        const auto row1 =
            generate_zmap(m_diff[DFA::FaultIndex[group][1]], DFA::mix_columns_matrix[faultrow][1]);
        const auto row2 =
            generate_zmap(m_diff[DFA::FaultIndex[group][2]], DFA::mix_columns_matrix[faultrow][2]);
        const auto row3 =
            generate_zmap(m_diff[DFA::FaultIndex[group][3]], DFA::mix_columns_matrix[faultrow][3]);

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
                    candidate.add(DFA::forward_box[j] ^ m_ref[DFA::FaultIndex[group][0]], 0);
                }

                if (row1[j] == i)
                {
                    candidate.add(DFA::forward_box[j] ^ m_ref[DFA::FaultIndex[group][1]], 1);
                }

                if (row2[j] == i)
                {
                    candidate.add(DFA::forward_box[j] ^ m_ref[DFA::FaultIndex[group][2]], 2);
                }

                if (row3[j] == i)
                {
                    candidate.add(DFA::forward_box[j] ^ m_ref[DFA::FaultIndex[group][3]], 3);
                }
            }
        }
    }

    Fault m_fault, m_ref, m_diff;
};
