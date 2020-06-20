#include "u128.h"

#include <algorithm>

u128 xor128(const u128& f1, const u128& f2)
{
    u128 xord;

    const auto xorByte = [](auto a, auto b) { return a ^ b; };
    std::transform(f1.begin(), f1.end(), f2.begin(), xord.begin(), xorByte);

    return xord;
}
