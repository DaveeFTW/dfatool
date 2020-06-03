#include "dfa.h"

#include <immintrin.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace
{
    enum class Mode
    {
        Encrypt,
        Decrypt
    };

    enum class Type
    {
        Round8,
        Round9
    };

    std::optional<std::vector<__m128i>> readFaults(const std::string& path)
    {
        std::ifstream file{ path };

        if (!file.is_open())
        {
            std::cerr << "error opening file \"" << path << "\" for reading." << std::endl;
            return {};
        }

        std::vector<__m128i> faults;
        std::string line;

        while (std::getline(file, line))
        {
            std::array<uint8_t, 16> binaryData = { 0 };
            auto nibblesRead = 0;

            for (auto c : line)
            {
                c = toupper(c);

                if (isspace(c))
                {
                    continue;
                }

                if (nibblesRead == binaryData.size() * 2)
                {
                    std::cerr << "too many characters to form 128 bit hexadecimal fault. "
                                 "Unexpected character:\""
                              << c << "\"" << std::endl;
                    return {};
                }

                constexpr std::array hexDigits = { '0', '1', '2', '3', '4', '5', '6', '7',
                                                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

                const auto it = std::find(hexDigits.begin(), hexDigits.end(), c);

                if (it == hexDigits.end())
                {
                    std::cerr << "unrecognised hex character \"" << c << "\" found in file."
                              << std::endl;
                    return {};
                }

                const auto value = std::distance(hexDigits.begin(), it);
                binaryData[nibblesRead / 2] |= (nibblesRead % 2) ? (value) : (value << 4);
                nibblesRead++;
            }

            faults.push_back(_mm_loadu_si128(reinterpret_cast<const __m128i*>(binaryData.data())));
        }

        return faults;
    }

    template<typename DFA>
    struct SolveR8
    {
        static constexpr auto solve = solve_r8<DFA>;
    };

    template<typename DFA>
    struct SolveR9
    {
        static constexpr auto solve = solve_r9<DFA>;
    };

    template<typename Solver>
    bool solve(const std::vector<__m128i>& faults)
    {
        for (auto& ref : faults)
        {
            if (Solver::solve(faults, ref))
                return true;
        }

        return false;
    }

    template<typename DFA>
    bool solve(Type type, const std::vector<__m128i>& faults)
    {
        switch (type)
        {
            case Type::Round8:
                return solve<SolveR8<DFA>>(faults);

            case Type::Round9:
                return solve<SolveR9<DFA>>(faults);
        }
    }

    bool solve(Mode mode, Type type, const std::vector<__m128i>& faults)
    {
        switch (mode)
        {
            case Mode::Encrypt:
                return solve<EncryptDFA>(type, faults);

            case Mode::Decrypt:
                return solve<DecryptDFA>(type, faults);
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        std::cerr << "usage: dfatool --encrypt/--decrypt [--r8/--r9] file" << std::endl;
        return 1;
    }

    std::string modeStr{ argv[1] };
    std::string typeStr{ argc == 4 ? argv[2] : "--r8" };
    std::string filePath{ argv[argc - 1] };

    Mode mode{ Mode::Encrypt };

    if (modeStr == "--encrypt")
    {
        mode = Mode::Encrypt;
    }

    else if (modeStr == "--decrypt")
    {
        mode = Mode::Decrypt;
    }

    else
    {
        std::cerr << "missing DFA encryption mode. use \"--encrypt\" or \"--decrypt\"" << std::endl;
        return 1;
    }

    Type type{ Type::Round8 };

    if (typeStr == "--r8")
    {
        type = Type::Round8;
    }

    else if (typeStr == "--r9")
    {
        type = Type::Round9;
    }

    else
    {
        std::cerr << "missing DFA fault type. use \"--r8\" or \"--r9\"" << std::endl;
        return 1;
    }

    const auto faults = readFaults(filePath);

    if (!faults)
    {
        return 1;
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    auto solution = solve(mode, type, faults.value());

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    if (!solution)
    {
        std::cout << "failed: could not find a key" << std::endl;
    }

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;
}
