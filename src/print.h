#pragma once

#include <immintrin.h>

void print_as_hex_matrix(const char* prefix, const __m128i value);
void print_as_hex(const char* prefix, const __m128i value);
