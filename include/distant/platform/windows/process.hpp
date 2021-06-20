#pragma once

#include <cstddef>
#include <cstdint>

namespace distant::platform {
    struct VirtualBuffer {
        struct Config {
            void *        desired_location = nullptr;
            std::uint32_t protect_flags    = 0x40; // PAGE_EXECUTE_READWRITE
        };
        void * parent_process_handle = nullptr;
    };

    struct Module {
        void *      base_address          = nullptr;
        std::size_t base_size             = 0;
        void *      parent_process_handle = nullptr;
    };

    struct Process {
        std::uint32_t id     = ~std::uint32_t{0};
        void *        handle = nullptr;
    };
} // namespace distant::platform
