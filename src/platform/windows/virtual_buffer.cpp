#include <cstdio>
#include <distant/core.hpp>
#include <format>

namespace distant {
    UniqueVirtualBuffer::UniqueVirtualBuffer(
        const Process & parent_process, std::size_t buffer_size,
        const platform::VirtualBuffer::Config & config)
        : native{parent_process.native.handle}, size{buffer_size} {
        address = std::bit_cast<Address>(VirtualAllocEx(
            native.parent_process_handle, config.desired_location, buffer_size,
            MEM_COMMIT | MEM_RESERVE, config.protect_flags));
    }

    UniqueVirtualBuffer::~UniqueVirtualBuffer() {
        if (static_cast<bool>(*this))
            VirtualFreeEx(native.parent_process_handle,
                          std::bit_cast<LPVOID>(address), 0, MEM_RELEASE);
    }

    UniqueVirtualBuffer::UniqueVirtualBuffer(
        UniqueVirtualBuffer && other) noexcept {
        native  = other.native;
        address = other.address;
        size    = other.size;

        other.native  = {};
        other.address = std::bit_cast<Address>(nullptr);
        other.size    = 0;
    }

    UniqueVirtualBuffer &
    UniqueVirtualBuffer::operator=(UniqueVirtualBuffer && other) noexcept {
        native  = other.native;
        address = other.address;
        size    = other.size;

        other.native  = {};
        other.address = std::bit_cast<Address>(nullptr);
        other.size    = 0;

        return *this;
    }

    UniqueVirtualBuffer::operator bool() const {
        return std::bit_cast<LPVOID>(address) != nullptr;
    }

    bool UniqueVirtualBuffer::read_buffer(AddressOffset offset,
                                          void *        buffer_ptr,
                                          std::size_t   buffer_size) const {
        return ReadProcessMemory(
            native.parent_process_handle,
            std::bit_cast<LPVOID>(std::bit_cast<std::intptr_t>(address) +
                                  offset),
            buffer_ptr, buffer_size, nullptr);
    }

    bool UniqueVirtualBuffer::write_buffer(AddressOffset offset,
                                           const void *  buffer_ptr,
                                           std::size_t   buffer_size) const {
        return WriteProcessMemory(
            native.parent_process_handle,
            std::bit_cast<LPVOID>(std::bit_cast<std::intptr_t>(address) +
                                  offset),
            buffer_ptr, buffer_size, nullptr);
    }
} // namespace distant
