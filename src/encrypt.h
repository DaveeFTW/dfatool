#pragma once

#include "aestools.h"

#include <array>

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

    static constexpr std::uint8_t mix_columns_matrix[4][4] = { { 2, 1, 1, 3 },
                                                               { 3, 2, 1, 1 },
                                                               { 1, 3, 2, 1 },
                                                               { 1, 1, 3, 2 } };
};

std::array<std::uint8_t, 256> EncryptDFA::inverse_box = { aes_isbox };
std::array<std::uint8_t, 256> EncryptDFA::forward_box = { aes_sbox };
