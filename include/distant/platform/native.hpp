#pragma once

#if defined(_WIN32) || defined(_WIN64)

#define DISTANT_PLATFORM_WINDOWS
#include <distant/platform/windows/injection.hpp>
#include <distant/platform/windows/process.hpp>
#if defined(DISTANT_EXPOSE_NATIVE)
#include <distant/platform/windows/native.hpp>
#endif

#else

#error "[ERROR] Unsupported Platform!"

#endif
