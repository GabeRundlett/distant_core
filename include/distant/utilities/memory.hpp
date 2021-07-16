#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace distant {
    // Address is a strict type meant to represent an
    // address or pointer in the global address space,
    // thus meaning it is not relative to any process
    // or module.
    using Address = std::intptr_t;

    // AddressOffset is a strict type meant to represent
    // a pointer offset, relative to some object with its
    // own (global or relative) address.
    using AddressOffset = std::intptr_t;

    // RelativeAddressSpace is a type intended to be
    // inherited. It has no data members, and just provides
    // CRTP function wrappers to allow reading/writing inside
    // some address space using relative offsets
    template <typename BaseT> struct RelativeAddressSpace {
        template <typename T> T read_value(AddressOffset offset) const {
            T result;
            static_cast<const BaseT &>(*this).read(offset, &result, sizeof(T));
            return result;
        }
        auto read_buffer(AddressOffset offset, std::size_t count) const {
            std::vector<std::uint8_t> result;
            result.resize(count);
            if (!static_cast<const BaseT &>(*this).read(offset, result.data(),
                                                        count))
                return std::vector<std::uint8_t>{};
            return result;
        }
        template <typename T>
        bool write_value(AddressOffset offset, const T & value) const {
            return static_cast<const BaseT &>(*this).write(offset, &value,
                                                           sizeof(T));
        }
    };

    // AbsoluteAddressSpace is a type intended to be
    // inherited. It has no data members, and just provides
    // CRTP function wrappers to allow reading/writing inside
    // some address space using absolute addresses
    template <typename BaseT> struct AbsoluteAddressSpace {
        template <typename T> T read_value(Address address) const {
            T result;
            static_cast<const BaseT &>(*this).read(address, &result, sizeof(T));
            return result;
        }
        auto read_buffer(Address address, std::size_t count) const {
            std::vector<std::uint8_t> result;
            result.resize(count);
            if (!static_cast<const BaseT &>(*this).read(address, result.data(),
                                                        count))
                return std::vector<std::uint8_t>{};
            return result;
        }
        template <typename T>
        bool write_value(Address address, const T & value) const {
            return static_cast<const BaseT &>(*this).write(address, &value,
                                                           sizeof(T));
        }
    };
} // namespace distant
