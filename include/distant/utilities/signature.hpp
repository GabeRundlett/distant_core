#pragma once

#include <algorithm>
#include <array>

namespace distant {
    // SignatureEntry is a byte and flags pair that allows
    // one to hold a collection of entries that represent
    // a signature pattern in memory. One can easily create
    // an array of these themselves, but the intended usage
    // is to construct an array using the C++20 user defined
    // literals provided, ie. _IdaSig and _CodeSig.
    struct SignatureEntry {
        std::uint8_t byte = 0x00;
        struct Flags {
            bool           wildcard : 1                             = false;
            constexpr bool operator==(const Flags &) const noexcept = default;
        } flags;
        static_assert(sizeof(Flags) == sizeof(std::uint8_t));
        constexpr bool
        operator==(const SignatureEntry &) const noexcept = default;
    };

    namespace detail {
        // Implementation Detail [not meant to be used]
        constexpr std::uint8_t ascii_to_hex(char c) {
            if (c >= '0' && c <= '9') { return std::uint8_t(c) - '0'; }
            if (c >= 'a' && c <= 'f') { return std::uint8_t(c) - 'a' + 10; }
            if (c >= 'A' && c <= 'F') { return std::uint8_t(c) - 'A' + 10; }
            return 0;
        }

        // Implementation Detail [not meant to be used]
        constexpr std::uint8_t ida_signature_byte(char c1, char c2) {
            return (ascii_to_hex(c1) << 4) + ascii_to_hex(c2);
        }

        // Implementation Detail [not meant to be used]
        template <std::size_t CHAR_N> struct PatternString {
            std::array<char, CHAR_N> chars;
            constexpr PatternString(const char (&in)[CHAR_N]) : chars{} {
                std::copy(in, in + CHAR_N, chars.begin());
            }
        };
    } // namespace detail
} // namespace distant

namespace distant::signature_literals {
    // The `_IdaSig` user defined literal constructs
    // a std::array<SignatureEntry, N> where N is the
    // parsed number of pattern bytes in the provided
    // string literal.
    // It requires the string literal is well formed in
    // the IDA signature format, for example
    //   "f0 2E ? 16"
    // would be converted to an array of SignatureEntrys
    // with data like:
    //   {0xf0}, {0x2e}, {wild}, {0x16}
    template <distant::detail::PatternString SIG_STR>
    consteval auto operator""_IdaSig() {
        constexpr std::size_t ENTRY_N =
            std::ranges::count(SIG_STR.chars, ' ') + 1;
        std::array<SignatureEntry, ENTRY_N> result = {};
        for (std::size_t str_i = 0, i = 0; str_i < SIG_STR.chars.size() - 1;
             ++str_i) {
            auto c = SIG_STR.chars[str_i];
            if (c != ' ') {
                if (c == '?') {
                    result[i] = {.byte = 0, .flags{.wildcard = true}};
                } else {
                    result[i] = {
                        .byte = detail::ida_signature_byte(
                            c, SIG_STR.chars[str_i + 1]),
                        .flags{.wildcard = false},
                    };
                    ++str_i;
                }
                ++i;
            }
        }
        return result;
    }

    // The `_CodeSig` user defined literal constructs
    // a std::array<SignatureEntry, N> where N is the
    // parsed number of pattern bytes in the provided
    // string literal.
    // It requires the string literal is well formed in
    // the Code signature format, for example
    //   "\xF0\x2e\x00\x16 xx?x"
    // would be converted to an array of SignatureEntrys
    // with data like:
    //   {0xf0}, {0x2e}, {wild}, {0x16}
    template <distant::detail::PatternString SIG_STR>
    consteval auto operator""_CodeSig() {
        constexpr std::size_t ENTRY_N = SIG_STR.chars.size() / 2 - 1;
        std::array<SignatureEntry, ENTRY_N> result = {};
        for (int i = 0; i < ENTRY_N; ++i) {
            std::uint8_t byte = SIG_STR.chars[i];
            auto         wild = SIG_STR.chars[i + ENTRY_N + 1];
            if (wild == '?') {
                result[i] = {
                    .byte = 0,
                    .flags{.wildcard = true},
                };
            } else {
                result[i] = {
                    .byte = byte,
                    .flags{.wildcard = false},
                };
            }
        }
        return result;
    }

    static_assert("f0 2E ? 16"_IdaSig == "\xF0\x2e\x00\x16 xx?x"_CodeSig);
} // namespace distant::signature_literals
