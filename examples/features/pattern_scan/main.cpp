#include <distant/core.hpp>
#include <distant/signature.hpp>

#include <iostream>
#include <format>

int main() {
    distant::Process external_process("Distant_TargetApps_Simple.exe");
    if (!external_process) { return EXIT_FAILURE; }
    auto base_module =
        external_process.find_module("Distant_TargetApps_Simple.exe");

    // read entire base module memory buffer to vector
    std::vector<std::uint8_t> module_memory_buffer;
    module_memory_buffer.resize(base_module.size());
    if (!base_module.read_buffer(0, module_memory_buffer.data(),
                                 module_memory_buffer.size())) {
        return EXIT_FAILURE;
    }

    using namespace distant::signature_literals;
    constexpr auto signature = "ff 05"_IdaSig;

    bool                   found_signature = false;
    distant::AddressOffset signature_offset;
    for (std::size_t i = 0; i < module_memory_buffer.size(); ++i) {
        std::size_t j = 0;
        for (; j < signature.size(); ++j) {
            if (!signature[j].wildcard &&
                signature[j].byte != module_memory_buffer[i + j])
                break;
        }
        if (j == signature.size()) {
            found_signature  = true;
            signature_offset = i;
            break;
        }
    }

    if (!found_signature) {
        std::cout << "Failed to find signature\n";
        return EXIT_FAILURE;
    }

    std::cout << std::format("Signature found at offset {}\n",
                             (void *)signature_offset);

    // write 6 bytes to 0x90, AKA no-op. This disables the `inc` instruction
    // and its 4 byte argument, allowing the code to continue perfectly fine
    for (int i = 0; i < 6; ++i) {
        base_module.write<std::uint8_t>(signature_offset + i, 0x90);
    }

    return EXIT_SUCCESS;
}
