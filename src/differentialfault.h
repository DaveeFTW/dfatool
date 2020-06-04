#pragma once

#include "fault.h"

template<typename DFA>
class DifferentialFault
{
public:
    DifferentialFault(const Fault& fault, const Fault& reference)
        : m_ref{ reference }
        , m_diff{ xor128(fault, m_ref) }
    {
    }

    bool is_group_affected(int group) const
    {
        const auto row0diff = m_diff[DFA::FaultIndex[group][0]];
        const auto row1diff = m_diff[DFA::FaultIndex[group][1]];
        const auto row2diff = m_diff[DFA::FaultIndex[group][2]];
        const auto row3diff = m_diff[DFA::FaultIndex[group][3]];
        const auto affected = row0diff != 0 && row1diff != 0 && row2diff != 0 && row3diff != 0;
        return affected;
    }

    FaultCandidateList candidates_for_group(int group) const
    {
        FaultCandidateList candidates;
        candidates_for_fault(candidates, group, 0);
        candidates_for_fault(candidates, group, 1);
        candidates_for_fault(candidates, group, 2);
        candidates_for_fault(candidates, group, 3);
        return candidates;
    }

private:
    struct ZMap
    {
        constexpr auto intersectionForRow(unsigned int row) const { return (1u << row); }

        constexpr auto allIntersected() const
        {
            return intersection == (intersectionForRow(0) | intersectionForRow(1) |
                                    intersectionForRow(2) | intersectionForRow(3));
        }

        void addIntersection(int row) { intersection |= intersectionForRow(row); }

        unsigned int intersection{ 0 };
        FaultCandidate candidate;
    };

    auto generate_x_to_z_map(std::uint8_t diff, std::uint8_t multiplier) const
    {
        // clang-format off
        // we have the equation:
        // Y = S(X) + S(C*Z + X)
        // where Y is our diff, C is our multiplier, Z is our fault, and X is some unknown AES
        // state we can rework this:
        // Y = S(X) + S(C*Z + X)
        // S(C*Z + X) = Y + S(X)
        // C*Z + X = InvSBox(Y + S(X))
        // C*Z = InvSBox(Y + S(X)) + X
        // Z = InvMult(C)*(InvSBox(Y + S(X)) + X)
        // clang-format on

        std::array<std::uint8_t, 256> xzmap;

        // build a mapping of all possible unknowns (by index) to possible Z values.
        for (auto X = 0u; X < xzmap.size(); ++X)
        {
            xzmap[X] = aes_gf_imult[multiplier][DFA::inverse_box[diff ^ DFA::forward_box[X]] ^ X];
        }

        return xzmap;
    }

    void candidates_for_fault(FaultCandidateList& candidates, int group, int faultrow) const
    {
        const std::array rows = { generate_x_to_z_map(m_diff[DFA::FaultIndex[group][0]],
                                                      DFA::mix_columns_matrix[faultrow][0]),
                                  generate_x_to_z_map(m_diff[DFA::FaultIndex[group][1]],
                                                      DFA::mix_columns_matrix[faultrow][1]),
                                  generate_x_to_z_map(m_diff[DFA::FaultIndex[group][2]],
                                                      DFA::mix_columns_matrix[faultrow][2]),
                                  generate_x_to_z_map(m_diff[DFA::FaultIndex[group][3]],
                                                      DFA::mix_columns_matrix[faultrow][3]) };
        static_assert(rows.size() == 4);

        // in reality there are only 127 unique values of Z. We use 256 values to cover the entire
        // range of an 8 bit integer so we can cleanly index the Z values into this array.
        std::array<ZMap, 256> zmap;

        // now we combine all these values
        for (auto X = 0u; X < 256u; ++X)
        {
            for (auto row = 0u; row < rows.size(); ++row)
            {
                auto& Z = zmap[rows[row][X]];
                Z.addIntersection(row);

                // store the X values into the candidate for the row they were found in
                Z.candidate.add(X, row);
            }
        }

        for (const auto& z : zmap)
        {
            // we only care about z values that appear in each row
            if (!z.allIntersected())
            {
                continue;
            }

            candidates.candidates.push_back(z.candidate);
        }
    }

    const Fault m_ref, m_diff;
};
