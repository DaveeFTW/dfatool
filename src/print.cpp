#include "print.h"

#include <array>
#include <cstdio>

void print_as_hex_matrix(const char* prefix, const __m128i value)
{
    std::array<uint8_t, 16> data;
    _mm_storeu_si128(reinterpret_cast<__m128i*>(data.data()), value);

    printf("%s\n"
           "%02X %02X %02X %02X\n"
           "%02X %02X %02X %02X\n"
           "%02X %02X %02X %02X\n"
           "%02X %02X %02X %02X\n",
           prefix,
           data.data()[0],
           data.data()[4],
           data.data()[8],
           data.data()[12],
           data.data()[1],
           data.data()[5],
           data.data()[9],
           data.data()[13],
           data.data()[2],
           data.data()[6],
           data.data()[10],
           data.data()[14],
           data.data()[3],
           data.data()[7],
           data.data()[11],
           data.data()[15]);
}

void print_as_hex(const char* prefix, const __m128i value)
{
    std::array<uint8_t, 16> data;
    _mm_storeu_si128(reinterpret_cast<__m128i*>(data.data()), value);

    printf("%s %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
           prefix,
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
}
