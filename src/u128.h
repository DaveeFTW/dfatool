#pragma once

#include <array>
#include <iostream>
using u128 = std::array<std::uint8_t, 16>;

u128 xor128(const u128& f1, const u128& f2);
std::ostream& operator<<(std::ostream& out, const u128& f);