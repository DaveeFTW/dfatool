#include "u128.h"

#include <algorithm>

u128 xor128(const u128& f1, const u128& f2)
{
    u128 xord;

    const auto xorByte = [](auto a, auto b) { return a ^ b; };
    std::transform(f1.begin(), f1.end(), f2.begin(), xord.begin(), xorByte);

    return xord;
}

std::ostream& operator<<(std::ostream& out, const u128& f)
{
    return out << static_cast<unsigned int>(f.data()[0]) << static_cast<unsigned int>(f.data()[1])
               << static_cast<unsigned int>(f.data()[2]) << static_cast<unsigned int>(f.data()[3])
               << static_cast<unsigned int>(f.data()[4]) << static_cast<unsigned int>(f.data()[5])
               << static_cast<unsigned int>(f.data()[6]) << static_cast<unsigned int>(f.data()[7])
               << static_cast<unsigned int>(f.data()[8]) << static_cast<unsigned int>(f.data()[9])
               << static_cast<unsigned int>(f.data()[10]) << static_cast<unsigned int>(f.data()[11])
               << static_cast<unsigned int>(f.data()[12]) << static_cast<unsigned int>(f.data()[13])
               << static_cast<unsigned int>(f.data()[14])
               << static_cast<unsigned int>(f.data()[15]);
}
