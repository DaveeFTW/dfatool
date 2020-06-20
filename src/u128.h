#pragma once

#include <array>

using u128 = std::array<std::uint8_t, 16>;

u128 xor128(const u128& f1, const u128& f2);
