#include <distant/core.hpp>
#include <distant/signature.hpp>

#include <iostream>
#include <format>

int main() {
    distant::Process external_process("Distant_TargetApps_Simple.exe");
    if (!external_process) { return EXIT_FAILURE; }
    auto base_module =
        external_process.find_module("Distant_TargetApps_Simple.exe");

    auto module_memory_buffer = base_module.read_buffer(0, base_module.size());
    if (module_memory_buffer.size() != base_module.size()) {
        return EXIT_FAILURE;
    }

    using namespace distant::signature_literals;
    constexpr auto signature = "ff 05"_IdaSig;

    bool                   found_signature = false;
    distant::AddressOffset signature_offset;

    std::size_t i = 0;
    for (; i < module_memory_buffer.size(); ++i) {
        std::size_t j = 0;
        for (; j < signature.size(); ++j) {
            if (!signature[j].flags.wildcard &&
                signature[j].byte != module_memory_buffer[i + j])
                break;
        }
        if (j == signature.size()) {
            found_signature  = true;
            signature_offset = i;
            break;
        }
    }

    std::cout << std::format(
        "Checked (dec {0}, hex {0:x}) bytes of the base module\n", i);

    if (!found_signature) {
        std::cout << "Failed to find signature\n";
        return EXIT_FAILURE;
    }

    std::cout << std::format("Signature found at offset {}\n",
                             (void *)signature_offset);

    auto code_bytes = base_module.read_buffer(signature_offset, 64);
    if (code_bytes.size() != 64) { return EXIT_FAILURE; }

    // INSTRUX ix;
    // if (!ND_SUCCESS(NdDecodeEx(&ix, code_bytes.data(), code_bytes.size(),
    // ND_CODE_64, ND_DATA_64))) {
    //     return EXIT_FAILURE;
    // }

    return EXIT_SUCCESS;
}
