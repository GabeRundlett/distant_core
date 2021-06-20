#pragma once

#include <distant/process.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace distant {
    class Injectable {
#if defined(DISTANT_EXPOSE_NATIVE)
      public:
#endif
        platform::Injectable native;

      public:
        std::vector<std::byte> binary;

        explicit Injectable(const std::filesystem::path & filepath);
        ~Injectable() = default;

        Injectable(const Injectable &) = delete;
        Injectable & operator=(const Injectable &) = delete;
        Injectable(Injectable &&) noexcept;
        Injectable & operator=(Injectable &&) noexcept;

        explicit operator bool() const;

        friend class InjectionContext;
    };

    class InjectionContext {
      private:
        UniqueVirtualBuffer image_buffer;
        const Process &     process;

      public:
        InjectionContext(const Process &    process,
                         const Injectable & injectable);
        ~InjectionContext() = default;

        InjectionContext(const InjectionContext &) = delete;
        InjectionContext & operator=(const InjectionContext &) = delete;
        InjectionContext(InjectionContext &&) noexcept;
        InjectionContext & operator=(InjectionContext &&) noexcept;

        explicit operator bool() const noexcept;

        void launch_thread();
    };
} // namespace distant
