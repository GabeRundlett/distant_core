#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define DISTANT_PLATFORM_WINDOWS
#include <distant/platform/windows/injection.hpp>
#else
#error "[ERROR] Unsupported Platform!"
#endif

#include <distant/process.hpp>

#include <string>
#include <vector>
#include <filesystem>

namespace distant {
    struct Injectable {
        platform::Injectable native;
        std::vector<std::byte> binary;

        Injectable(const std::filesystem::path &filepath);
        ~Injectable();

        Injectable(const Injectable &) = delete;
        Injectable(Injectable &&) noexcept;
        Injectable &operator=(const Injectable &) = delete;
        Injectable &operator=(Injectable &&) noexcept;

        operator bool() const;
    };

    struct InjectionContext {
        platform::InjectionContext native;
        UniqueVirtualBuffer image_buffer;
        const Process &process;

        InjectionContext(const Process &process, const Injectable &injectable);
        ~InjectionContext();

        InjectionContext(const InjectionContext &) = delete;
        InjectionContext(InjectionContext &&) noexcept;
        InjectionContext &operator=(const InjectionContext &) = delete;
        InjectionContext &operator=(InjectionContext &&) noexcept;

        operator bool() const;

        void launch_thread();
    };
} // namespace distant
