#pragma once

#include <distant/memory.hpp>
#include <distant/platform/native.hpp>
#include <optional>
#include <string>

namespace distant {
    class Process;

    // Internally, basically a pointer to heap memory owned
    // by an external process. It also stores any reference
    // necessary to the process inside the .native member in
    // order to allow automatic freeing upon destruction, as
    // well as the size of the buffer.
    class UniqueVirtualBuffer
        : public RelativeAddressSpace<UniqueVirtualBuffer> {
#if defined(DISTANT_EXPOSE_NATIVE)
      public:
#endif
        platform::VirtualBuffer native = {};

      private:
        Address     address;
        std::size_t size;

      public:
        UniqueVirtualBuffer(
            const Process & parent_process, std::size_t buffer_size,
            const platform::VirtualBuffer::Config & config = {});
        ~UniqueVirtualBuffer();

        UniqueVirtualBuffer(const UniqueVirtualBuffer &) = delete;
        UniqueVirtualBuffer(UniqueVirtualBuffer &&) noexcept;
        UniqueVirtualBuffer & operator=(const UniqueVirtualBuffer &) = delete;
        UniqueVirtualBuffer & operator=(UniqueVirtualBuffer &&) noexcept;

        explicit operator bool() const;

        bool read_buffer(AddressOffset offset, void * buffer_ptr,
                         std::size_t buffer_size) const;
        bool write_buffer(AddressOffset offset, const void * buffer_ptr,
                          std::size_t buffer_size) const;
        constexpr Address get() const { return address; }
    };

    // An object representing a running processes individual
    // components, such as (On Windows) the .exe module in
    // memory or kernel.dll loaded by the OS, and (on Linux)
    // the ... TODO Add Linux Support
    class Module : public RelativeAddressSpace<Module> {
#if defined(DISTANT_EXPOSE_NATIVE)
      public:
#endif
        platform::Module native;

      public:
        Module(const Process & parent_process, const std::string & module_name);
        ~Module();

        Module(const Module &) = delete;
        Module(Module &&) noexcept;
        Module & operator=(const Module &) = delete;
        Module & operator                  =(Module &&) noexcept;

        Address get_base_address() const;
        bool    read_buffer(AddressOffset offset, void * buffer_ptr,
                            std::size_t buffer_size) const;
        bool    write_buffer(AddressOffset offset, const void * buffer_ptr,
                             std::size_t buffer_size) const;

        std::size_t size() const noexcept;
        std::byte * data() const noexcept;
    };

    // An attachment to a running process, allowing for reading
    // and writing inside the processes address space.
    class Process : public AbsoluteAddressSpace<Process> {
#if defined(DISTANT_EXPOSE_NATIVE)
      public:
#endif
        platform::Process native;

      public:
        explicit Process(const std::string & process_name);
        ~Process();

        Process(const Process &) = delete;
        Process(Process &&) noexcept;
        Process & operator=(const Process &) = delete;
        Process & operator                   =(Process &&) noexcept;

        explicit operator bool() const;

        UniqueVirtualBuffer virtual_alloc_unique(
            std::size_t                             buffer_size,
            const platform::VirtualBuffer::Config & config = {}) const;
        Module find_module(const std::string & module_name) const;

        bool read_buffer(Address address, void * buffer_ptr,
                         std::size_t buffer_size) const;
        bool write_buffer(Address address, const void * buffer_ptr,
                          std::size_t buffer_size) const;

        friend class UniqueVirtualBuffer;
        friend class Module;
        friend class InjectionContext;
    };
} // namespace distant
