#include <distant/core.hpp>

namespace targetapp {
    constexpr distant::AddressOffset VALUE_OFFSET      = 0x5000;
    constexpr distant::AddressOffset SERVER_PTR_OFFSET = 0x50F0;
}; // namespace targetapp

int main() {
    using namespace targetapp;

    auto external_process = distant::Process("Distant_TargetApps_Simple.exe");
    auto base_module =
        external_process.find_module("Distant_TargetApps_Simple.exe");

    base_module.write<int>(VALUE_OFFSET, 10);

    auto server_value_address =
        base_module.read<distant::Address>(SERVER_PTR_OFFSET);
    external_process.write<int>(server_value_address, 38);
}
