#pragma once

#include "aestools.h"

#include <array>

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
