#include <distant/core.hpp>
#include <cstdio>
#include <fmt/format.h>

namespace distant {
    static std::optional<PROCESSENTRY32> find_process_entry(const std::string &process_exe) {
        PROCESSENTRY32 result;
        result.dwSize = sizeof(result);

        HANDLE process_snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (process_snapshot_handle == INVALID_HANDLE_VALUE) {
            std::printf("%s", fmt::format("[ERROR:{:x}] Failed to create process snapshot\n", GetLastError()).c_str());
            return std::nullopt;
        }

        Process32First(process_snapshot_handle, &result);
        if (process_exe.compare(result.szExeFile) == 0) {
            CloseHandle(process_snapshot_handle);
            return result;
        }
        while (Process32Next(process_snapshot_handle, &result)) {
            if (process_exe.compare(result.szExeFile) == 0) {
                CloseHandle(process_snapshot_handle);
                return result;
            }
        }

        CloseHandle(process_snapshot_handle);
        return std::nullopt;
    }

    Process::Process(const std::string &process_exe) {
        native = {};
        auto process_entry = find_process_entry(process_exe);
        if (!process_entry.has_value()) {
            std::printf("%s", fmt::format("[ERROR:{:x}] Failed to find process '{}'\n", GetLastError(), process_exe).c_str());
            return;
        }
        native = {.id = process_entry.value().th32ProcessID};
        native.handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, native.id);
        if (!native.handle) {
            std::printf("%s", fmt::format("[ERROR:{:x}] Failed to open process '{}'\n", GetLastError(), process_exe).c_str());
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

    Process::Process(Process &&other) noexcept {}
    Process &Process::operator=(Process &&other) noexcept { return *this; }
    Process::operator bool() const { return native.handle != nullptr; }

    UniqueVirtualBuffer Process::virtual_alloc_unique(std::size_t buffer_size, const platform::VirtualBuffer::Config &config) const {
        return UniqueVirtualBuffer(*this, buffer_size, config);
    }

    Module Process::find_module(const std::string &module_name) const {
        return Module(*this, module_name);
    }

    bool Process::read_buffer(Address address, void *buffer_ptr, std::size_t buffer_size) const {
        if (!ReadProcessMemory(native.handle, std::bit_cast<LPVOID>(address), buffer_ptr, buffer_size, nullptr)) {
            std::printf("%s", fmt::format("[ERROR:{:x}] Failed reading process memory at address {} with size {}\n", GetLastError(), (void *)address, buffer_size).c_str());
            return false;
        }
        return true;
    }

    bool Process::write_buffer(Address address, const void *buffer_ptr, std::size_t buffer_size) const {
        if (!WriteProcessMemory(native.handle, std::bit_cast<LPVOID>(address), buffer_ptr, buffer_size, nullptr)) {
            std::printf("%s", fmt::format("[ERROR:{:x}] Failed writing process memory at address {} with size {}\n", GetLastError(), (void *)address, buffer_size).c_str());
            return false;
        }
        return true;
    }
} // namespace distant
