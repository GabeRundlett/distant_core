#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <TlHelp32.h>

namespace distant::platform {
    struct VirtualBuffer {
        struct Config {
            void *desired_location = nullptr;
            DWORD protect_flags = PAGE_EXECUTE_READWRITE;
        };
        HANDLE parent_process_handle = nullptr;
    };

    struct Module {
        void *base_address = nullptr;
        std::size_t size = 0;
        HANDLE parent_process_handle = nullptr;
    };

    struct Process {
        DWORD id = ~DWORD{0};
        HANDLE handle = nullptr;
    };
} // namespace distant::platform
