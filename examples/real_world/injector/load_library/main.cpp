#define DISTANT_EXPOSE_NATIVE
#include <distant/core.hpp>

#include <iostream>
#include <thread>
using namespace std::chrono_literals;

int main() {
    std::string process_name;
    process_name = "hello_app.exe";
    distant::Process process(process_name);
    if (!process) {
        std::cout << "Failed to attach to running process\n";
        return EXIT_FAILURE;
    }
    std::filesystem::path injectable_filepath;
    injectable_filepath =
        std::filesystem::current_path() /
        "build/examples/injectable_dlls/hello/Release/hello_dll.dll";
    auto str = injectable_filepath.string();
    auto injectable_filepath_remote_buffer =
        process.virtual_alloc_unique(str.size() + 1);
    injectable_filepath_remote_buffer.write_buffer(0, str.data(), str.size());
    injectable_filepath_remote_buffer.write(str.size(), '\0');
    auto kernel32_dll_handle = GetModuleHandle("kernel32.dll");
    if (!kernel32_dll_handle) {
        std::cout << "Failed to get \"kernel32.dll\" dll module handle\n";
        return EXIT_FAILURE;
    }
    auto load_library_address =
        GetProcAddress(kernel32_dll_handle, "LoadLibraryA");
    if (!load_library_address) {
        std::cout << "Failed to find \"LoadLibraryA\" function pointer\n";
        return EXIT_FAILURE;
    }
    auto thread_handle = CreateRemoteThread(
        process.native.handle, nullptr, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(load_library_address),
        std::bit_cast<void *>(injectable_filepath_remote_buffer.get()), 0,
        nullptr);
    if (!thread_handle) {
        std::cout << "Failed to create remote thread\n";
        return EXIT_FAILURE;
    }
    WaitForSingleObject(thread_handle, INFINITE);
    return EXIT_SUCCESS;
}
