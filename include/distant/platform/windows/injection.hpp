#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <TlHelp32.h>

namespace distant::platform {
    struct Injectable {
        IMAGE_DOS_HEADER *dos_header_ptr;
        IMAGE_NT_HEADERS *nt_headers_ptr;
        bool valid;
    };

    struct InjectionContext {
    };
} // namespace distant::platform
