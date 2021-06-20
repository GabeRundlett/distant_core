#pragma once

namespace distant::platform {
    struct Injectable {
        void * dos_header_ptr;
        void * nt_headers_ptr;
        bool   valid;
    };
} // namespace distant::platform
