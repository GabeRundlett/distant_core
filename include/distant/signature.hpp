#pragma once

#include <array>
#include <algorithm>

namespace distant {
    template <std::size_t StrLen>
    struct IdaSignatureString {
        std::array<char, StrLen> str_array{};
        std::int64_t byte_count = 0;
        consteval IdaSignatureString(const char (&str)[StrLen])
            : byte_count{std::ranges::count(str, ' ') + 1} {
            std::copy(str, str + StrLen, str_array.begin());
        }
    };

    template <std::size_t StrLen>
    struct CodeSignatureString {
        std::array<char, StrLen> str_array{};
        std::int64_t byte_count = 0;
        consteval CodeSignatureString(const char (&str)[StrLen])
            : byte_count{StrLen / 2 - 1} {
            std::copy(str, str + StrLen, str_array.begin());
        }
    };

    struct SignatureEntry {
        std::uint8_t byte = 0x00;
        bool wildcard : 1 = false;
    };
    constexpr bool operator==(const SignatureEntry &a, const SignatureEntry &b) {
        if (a.wildcard != b.wildcard)
            return false;
        return a.byte == b.byte;
    }

    namespace detail {
        consteval std::uint8_t ascii_to_hex(char c) {
            if (c >= '0' && c <= '9')
                return std::uint8_t(c) - '0';
            if (c >= 'a' && c <= 'f')
                return std::uint8_t(c) - 'a' + 10;
            if (c >= 'A' && c <= 'F')
                return std::uint8_t(c) - 'A' + 10;
            return 0;
        }

        consteval std::uint8_t ida_signature_byte(char c1, char c2) {
            return (ascii_to_hex(c1) << 4) + ascii_to_hex(c2);
        }
    } // namespace detail
} // namespace distant

namespace distant::signature_literals {
    template <IdaSignatureString ida_sig>
    consteval auto operator""_IdaSig() {
        std::array<SignatureEntry, ida_sig.byte_count> result;
        for (std::size_t str_i = 0, i = 0; str_i < ida_sig.str_array.size() - 1; ++str_i) {
            auto c = ida_sig.str_array[str_i];
            if (c != ' ') {
                if (c == '?') {
                    result[i] = {
                        .byte = 0,
                        .wildcard = true,
                    };
                } else {
                    result[i] = {
                        .byte = detail::ida_signature_byte(c, ida_sig.str_array[str_i + 1]),
                        .wildcard = false,
                    };
                    ++str_i;
                }
                ++i;
            }
        }
        return result;
        // return Signature<sig.byte_count>(sig);
    }

    template <CodeSignatureString code_sig>
    consteval auto operator""_CodeSig() {
        std::array<SignatureEntry, code_sig.byte_count> result;
        for (int i = 0; i < code_sig.byte_count; ++i) {
            std::uint8_t byte = code_sig.str_array[i];
            auto wild = code_sig.str_array[i + code_sig.byte_count + 1];

            if (wild == '?') {
                result[i] = {
                    .byte = 0,
                    .wildcard = true,
                };
            } else {
                result[i] = {
                    .byte = byte,
                    .wildcard = false,
                };
            }
        }
        return result;
        // return Signature<sig.byte_count>(sig);
    }

    static_assert("f0 2E ? 16"_IdaSig == "\xF0\x2e\x00\x16 xx?x"_CodeSig);
} // namespace distant::signature_literals
