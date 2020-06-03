#pragma once

#include "aestools.h"
#include "print.h"
#include "set.h"

#include <immintrin.h>

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

static inline __m128i rewind_r9_fault(__m128i state, __m128i key)
{
    // we want to reverse the encrypt stage rather than perform a decrypt
    // while similar, we must XOR the round key first, rather than last
    const auto xor_state = _mm_xor_si128(state, key);

    // we just want to reverse the mix column step only
    return _mm_aesimc_si128(xor_state);
}

template<int column, int row>
static inline __m128i generate_fault(__m128i r9, __m128i k9, __m128i k10, std::uint8_t fault)
{
    const auto mixed =
        _mm_xor_si128(mix_columns(_mm_insert_epi8(
                          r9, fault ^ _mm_extract_epi8(r9, column * 4 + row), column * 4 + row)),
                      k9);
    // print_as_hex("dmix:", mixed);
    return _mm_aesenclast_si128(mixed, k10);
}

static inline void print_fault(const char* msg,
                               __m128i fault,
                               __m128i ref,
                               __m128i key10,
                               __m128i key9)
{
    auto faultr9 = rewind_encrypt_last(fault, key10);
    auto refr9 = rewind_encrypt_last(ref, key10);

    auto imcfaultr9 = rewind_r9_fault(faultr9, key9);
    auto imcrefr9 = rewind_r9_fault(refr9, key9);

    print_as_hex_matrix(msg, _mm_xor_si128(imcfaultr9, imcrefr9));
}

static inline void print_fault_candidate(const FaultCandidate& candidate)
{
    const auto& v = candidate.values;

    printf("K0: {");

    bool first = true;

    for (auto r = 0; r < v[0].count; ++r)
    {
        if (!first)
        {
            printf(", ");
        }

        printf("%02X", v[0].values[r]);
        first = false;
    }

    printf("}, K1: {");

    first = true;

    for (auto r = 0; r < v[1].count; ++r)
    {
        if (!first)
        {
            printf(", ");
        }

        printf("%02X", v[1].values[r]);
        first = false;
    }

    printf("}, K2: {");

    first = true;

    for (auto r = 0; r < v[2].count; ++r)
    {
        if (!first)
        {
            printf(", ");
        }

        printf("%02X", v[2].values[r]);
        first = false;
    }

    printf("}, K3: {");

    first = true;

    for (auto r = 0; r < v[3].count; ++r)
    {
        if (!first)
        {
            printf(", ");
        }

        printf("%02X", v[3].values[r]);
        first = false;
    }

    printf("}\n");
}
