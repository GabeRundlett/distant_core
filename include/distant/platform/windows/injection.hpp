#pragma once

namespace distant::platform {
    constexpr auto INJECTABLE_ALIGNMENT = 32;
    struct alignas(INJECTABLE_ALIGNMENT) Injectable {
        void * dos_header_ptr;
        void * nt_headers_ptr;
        bool   valid;
    };
} // namespace distant::platform
