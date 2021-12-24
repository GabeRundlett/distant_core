#pragma once

#include <distant/memory.hpp>

#if defined(_WIN32) || defined(_WIN64)
#define DISTANT_PLATFORM_WINDOWS
#include <distant/platform/windows/process.hpp>
#else
#error "[ERROR] Unsupported Platform!"
#endif

#include <string>
#include <optional>

namespace distant {
    struct Process;

    // Internally, basically a pointer to heap memory owned
    // by an external process. It also stores any reference
    // necessary to the process inside the .native member in
    // order to allow automatic freeing upon destruction, as
    // well as the size of the buffer.
    struct UniqueVirtualBuffer : RelativeAddressSpace<UniqueVirtualBuffer> {
        platform::VirtualBuffer native = {};
        Address address;
        std::size_t size;

        UniqueVirtualBuffer() = default;
        UniqueVirtualBuffer(const Process &parent_process, std::size_t buffer_size, const platform::VirtualBuffer::Config &config = {});
        ~UniqueVirtualBuffer();

        UniqueVirtualBuffer(const UniqueVirtualBuffer &) = delete;
        UniqueVirtualBuffer(UniqueVirtualBuffer &&) noexcept;
        UniqueVirtualBuffer &operator=(const UniqueVirtualBuffer &) = delete;
        UniqueVirtualBuffer &operator=(UniqueVirtualBuffer &&) noexcept;

        operator bool() const;

        Address get_base_address() const;
        bool read_buffer(AddressOffset offset, void *buffer_ptr, std::size_t buffer_size) const;
        bool write_buffer(AddressOffset offset, const void *buffer_ptr, std::size_t buffer_size) const;
    };

    // An object representing a running processes individual
    // components, such as (On Windows) the .exe module in
    // memory or kernel.dll loaded by the OS, and (on Linux)
    // the ... TODO Add Linux Support
    struct Module : RelativeAddressSpace<Module> {
        platform::Module native;

        Module(const Process &parent_process, const std::string &module_name);
        ~Module();

        Module(const Module &) = delete;
        Module(Module &&) noexcept;
        Module &operator=(const Module &) = delete;
        Module &operator=(Module &&) noexcept;

        Address get_base_address() const { return reinterpret_cast<Address>(native.base_address); }
        bool read_buffer(AddressOffset offset, void *buffer_ptr, std::size_t buffer_size) const;
        bool write_buffer(AddressOffset offset, const void *buffer_ptr, std::size_t buffer_size) const;
    };

    // An attachment to a running process, allowing for reading
    // and writing inside the processes address space.
    struct Process : AbsoluteAddressSpace<Process> {
        platform::Process native;

        Process(const std::string &process_name);
        ~Process();

        Process(const Process &) = delete;
        Process(Process &&) noexcept;
        Process &operator=(const Process &) = delete;
        Process &operator=(Process &&) noexcept;

        operator bool() const;

        UniqueVirtualBuffer virtual_alloc_unique(std::size_t buffer_size, const platform::VirtualBuffer::Config &config = {}) const;
        Module find_module(const std::string &module_name) const;

        bool read_buffer(Address address, void *buffer_ptr, std::size_t buffer_size) const;
        bool write_buffer(Address address, const void *buffer_ptr, std::size_t buffer_size) const;
    };
} // namespace distant
