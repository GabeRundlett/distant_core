#include <distant/core.hpp>

int main() {
    distant::Process external_process("Distant_Features_TargetApp.exe");
    auto base_module = external_process.find_module("Distant_Features_TargetApp.exe");
    base_module.write<int>(distant::AddressOffset{0x5000}, 10);
    auto server_value_address = base_module.read<distant::Address>(0x50F0);
    external_process.write<int>(server_value_address, 38);
}
