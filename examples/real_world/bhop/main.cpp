#include <distant/core.hpp>
#include "hazedumper.hpp"
#include <iostream>

struct vec3 {
    float x, y, z;
};

struct player {
    char _pad1[112];
    vec3 pos;
};

int main() {
    distant::Process csgo_exe{"csgo.exe"};
    auto engine_dll = csgo_exe.find_module("engine.dll");
    auto client_dll = csgo_exe.find_module("client.dll");

    const auto client_state = engine_dll.read<ptrdiff_t>(hazedumper::signatures::dwClientState);
    const auto local_player_addr = client_dll.read<ptrdiff_t>(hazedumper::signatures::dwLocalPlayer)-client_dll.get_base_address();

    std::cout << "engine_dll = " << (void*)engine_dll.get_base_address() << std::endl;
    std::cout << "client_dll = " << (void*)client_dll.get_base_address() << std::endl;

    std::cout << "client_state = " << (void*)client_state << std::endl;
    std::cout << "local_player = " << (void*)local_player_addr << std::endl;
}
