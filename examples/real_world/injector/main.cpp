#include <distant/core.hpp>
#include <iostream>
#include <thread>
using namespace std::chrono_literals;

int main() {
    std::string process_name;
    // std::cout << "Enter process name: " << std::flush;
    // std::cin >> process_name;
    process_name = "target.exe";

    distant::Process process(process_name);
    if (!process) {
        std::cout << "Failed to attach to running process\n";
        return EXIT_FAILURE;
    }

    std::filesystem::path injectable_filepath;
    // std::cout << "Enter injectable name: " << std::flush;
    // std::cin >> injectable_filepath;
    injectable_filepath =
        std::filesystem::current_path() /
        "build/examples/injectable_dlls/basic/Release/basic.dll";

    distant::Injectable injectable(injectable_filepath);
    if (!injectable) {
        std::cout << "Failed to open injectable object\n";
        return EXIT_FAILURE;
    }

    distant::InjectionContext injection_context(process, injectable);
    if (!injection_context) {
        std::cout << "Failed to create injection context\n";
        return EXIT_FAILURE;
    }

    injection_context.launch_thread();
    return EXIT_SUCCESS;
}
