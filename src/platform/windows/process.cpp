#include <cstdio>
#include <distant/process.hpp>
#include <format>

// UniqueVirtualBuffer Implementation
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

    bool UniqueVirtualBuffer::read(AddressOffset offset, void * buffer_ptr,
                                   std::size_t buffer_size) const {
        return ReadProcessMemory(
            native.parent_process_handle,
            std::bit_cast<LPVOID>(std::bit_cast<std::intptr_t>(address) +
                                  offset),
            buffer_ptr, buffer_size, nullptr);
    }

    bool UniqueVirtualBuffer::write(AddressOffset offset,
                                    const void *  buffer_ptr,
                                    std::size_t   buffer_size) const {
        return WriteProcessMemory(
            native.parent_process_handle,
            std::bit_cast<LPVOID>(std::bit_cast<std::intptr_t>(address) +
                                  offset),
            buffer_ptr, buffer_size, nullptr);
    }
} // namespace distant

// Module Implementation
namespace distant {
    Module::Module(const Process &     parent_process,
                   const std::string & module_name) {
        HANDLE snapshot_handle = CreateToolhelp32Snapshot(
            TH32CS_SNAPMODULE, parent_process.native.id);
        MODULEENTRY32 module_entry{.dwSize = sizeof(MODULEENTRY32)};

        if (Module32First(snapshot_handle, &module_entry)) {
            while (true) {
                if (std::string(module_entry.szModule) == module_name) {
                    native.base_address          = module_entry.modBaseAddr;
                    native.base_size             = module_entry.modBaseSize;
                    native.parent_process_handle = parent_process.native.handle;
                    break;
                }

                if (!Module32Next(snapshot_handle, &module_entry)) break;
            }
        }

        CloseHandle(snapshot_handle);
    }

    Module::~Module() {}

    Module::Module(Module && other) noexcept {
        native       = other.native;
        other.native = {};
    }

    Module & Module::operator=(Module && other) noexcept {
        native       = other.native;
        other.native = {};

        return *this;
    }

    bool Module::read(AddressOffset offset, void * buffer_ptr,
                      std::size_t buffer_size) const {
        const auto global_address = std::bit_cast<LPVOID>(
            std::bit_cast<std::intptr_t>(native.base_address) + offset);
        if (!ReadProcessMemory(native.parent_process_handle, global_address,
                               buffer_ptr, buffer_size, nullptr)) {
            std::printf(std::format("[ERROR:{:x}] Failed reading module memory "
                                    "at offset {} (address {}) with size {}\n",
                                    GetLastError(), (void *)offset,
                                    global_address, buffer_size)
                            .c_str());
            return false;
        }
        return true;
    }

    bool Module::write(AddressOffset offset, const void * buffer_ptr,
                       std::size_t buffer_size) const {
        const auto global_address = std::bit_cast<LPVOID>(
            std::bit_cast<std::intptr_t>(native.base_address) + offset);
        if (!WriteProcessMemory(native.parent_process_handle, global_address,
                                buffer_ptr, buffer_size, nullptr)) {
            std::printf(std::format("[ERROR:{:x}] Failed writing module memory "
                                    "at offset {} (address {}) with size {}\n",
                                    GetLastError(), (void *)offset,
                                    global_address, buffer_size)
                            .c_str());
            return false;
        }
        return true;
    }

    std::size_t Module::size() const noexcept { return native.base_size; }
    std::byte * Module::data() const noexcept {
        return std::bit_cast<std::byte *>(native.base_address);
    }
} // namespace distant

// Process Implementation
namespace distant {
    static std::optional<PROCESSENTRY32>
    find_process_entry(const std::string & process_exe) {
        PROCESSENTRY32 result = {};
        result.dwSize         = sizeof(result);

        HANDLE process_snapshot_handle =
            CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (process_snapshot_handle == nullptr) {
            std::printf(
                std::format("[ERROR:{:x}] Failed to create process snapshot\n",
                            GetLastError())
                    .c_str());
            return std::nullopt;
        }

        Process32First(process_snapshot_handle, &result);
        if (process_exe == std::string(result.szExeFile)) {
            CloseHandle(process_snapshot_handle);
            return result;
        }
        while (Process32Next(process_snapshot_handle, &result)) {
            if (process_exe == std::string(result.szExeFile)) {
                CloseHandle(process_snapshot_handle);
                return result;
            }
        }

        CloseHandle(process_snapshot_handle);
        return std::nullopt;
    }

    Process::Process(const std::string & process_exe) {
        native             = {};
        auto process_entry = find_process_entry(process_exe);
        if (!process_entry.has_value()) {
            std::printf(
                std::format("[ERROR:{:x}] Failed to find process '{}'\n",
                            GetLastError(), process_exe)
                    .c_str());
            return;
        }
        native        = {.id = process_entry.value().th32ProcessID};
        native.handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, native.id);
        if (!native.handle) {
            std::printf(
                std::format("[ERROR:{:x}] Failed to open process '{}'\n",
                            GetLastError(), process_exe)
                    .c_str());
            return;
        }
    }

    Process::~Process() {
        if (!static_cast<bool>(*this)) {
            std::printf("Called destructor of invalid process\n");
            return;
        }
        CloseHandle(native.handle);
    }

    Process::Process(Process && other) noexcept {}
    Process & Process::operator=(Process && other) noexcept { return *this; }
    Process::operator bool() const { return native.handle != nullptr; }

    UniqueVirtualBuffer Process::virtual_alloc_unique(
        std::size_t                             buffer_size,
        const platform::VirtualBuffer::Config & config) const {
        return UniqueVirtualBuffer(*this, buffer_size, config);
    }

    Module Process::find_module(const std::string & module_name) const {
        return Module(*this, module_name);
    }

    bool Process::read(Address address, void * buffer_ptr,
                       std::size_t buffer_size) const {
        if (!ReadProcessMemory(native.handle, std::bit_cast<LPVOID>(address),
                               buffer_ptr, buffer_size, nullptr)) {
            std::printf(std::format("[ERROR:{:x}] Failed reading process "
                                    "memory at address {} with size {}\n",
                                    GetLastError(), (void *)address,
                                    buffer_size)
                            .c_str());
            return false;
        }
        return true;
    }

    bool Process::write(Address address, const void * buffer_ptr,
                        std::size_t buffer_size) const {
        if (!WriteProcessMemory(native.handle, std::bit_cast<LPVOID>(address),
                                buffer_ptr, buffer_size, nullptr)) {
            std::printf(std::format("[ERROR:{:x}] Failed writing process "
                                    "memory at address {} with size {}\n",
                                    GetLastError(), (void *)address,
                                    buffer_size)
                            .c_str());
            return false;
        }
        return true;
    }
} // namespace distant
