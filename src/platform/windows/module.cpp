#include <distant/core.hpp>
#include <cstdio>
#include <fmt/format.h>

namespace distant {
    Module::Module(const Process &parent_process, const std::string &module_name) {
        HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, parent_process.native.id);
        MODULEENTRY32 module_entry{.dwSize = sizeof(MODULEENTRY32)};

        if (Module32First(snapshot_handle, &module_entry)) {
            while (true) {
                if (std::string(module_entry.szModule) == module_name) {
                    native.base_address = module_entry.modBaseAddr;
                    native.base_size = module_entry.modBaseSize;
                    native.parent_process_handle = parent_process.native.handle;
                    break;
                }

                if (!Module32Next(snapshot_handle, &module_entry))
                    break;
            }
        }

        CloseHandle(snapshot_handle);
    }

    Module::~Module() {
    }

    Module::Module(Module &&other) noexcept {
        native = other.native;
        other.native = {};
    }

    Module &Module::operator=(Module &&other) noexcept {
        native = other.native;
        other.native = {};

        return *this;
    }

    bool Module::read_buffer(AddressOffset offset, void *buffer_ptr, std::size_t buffer_size) const {
        const auto global_address = std::bit_cast<LPVOID>(std::bit_cast<std::intptr_t>(native.base_address) + offset);
        if (!ReadProcessMemory(native.parent_process_handle, global_address, buffer_ptr, buffer_size, nullptr)) {
            std::printf("%s", fmt::format("[ERROR:{:x}] Failed reading module memory at offset {} (address {}) with size {}\n", GetLastError(), (void *)offset, global_address, buffer_size).c_str());
            return false;
        }
        return true;
    }

    bool Module::write_buffer(AddressOffset offset, const void *buffer_ptr, std::size_t buffer_size) const {
        const auto global_address = std::bit_cast<LPVOID>(std::bit_cast<std::intptr_t>(native.base_address) + offset);
        if (!WriteProcessMemory(native.parent_process_handle, global_address, buffer_ptr, buffer_size, nullptr)) {
            std::printf("%s", fmt::format("[ERROR:{:x}] Failed writing module memory at offset {} (address {}) with size {}\n", GetLastError(), (void *)offset, global_address, buffer_size).c_str());
            return false;
        }
        return true;
    }
} // namespace distant
