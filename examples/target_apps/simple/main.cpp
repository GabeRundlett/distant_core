#include <iostream>
#include <string>
#include <thread>
using namespace std::chrono_literals;

struct ApplicationInfo {
    int  value;
    bool should_quit : 1, should_change : 1;
};

int main() {
    static ApplicationInfo client_info{
        .value         = 4,
        .should_quit   = false,
        .should_change = true,
    };
    static auto server_info = std::make_unique<ApplicationInfo>(client_info);
    std::cout << "enter q to quit, enter h for help\n";

    while (!client_info.should_quit) {
        std::cout << "[Value = " << client_info.value << "]\n";

        char user_command_char;
        std::cin >> user_command_char;

        switch (user_command_char) {
        case 'q': client_info.should_quit = true; break;
        case 'h':
            std::cout << "Command list:\n"
                      << "  i: increment value\n"
                      << "  d: decrement value\n"
                      << "  r: randomize value\n"
                      << "  l: load server data\n"
                      << "  s: send server data\n"
                      << "  c: clear screen\n";
            break;
        case 'r':
            if (client_info.should_change) client_info.value = rand() % 10;
            break;
        case 'i': ++client_info.value; break;
        case 'd': --client_info.value; break;
        case 'l': client_info = *server_info; break;
        case 's': *server_info = client_info; break;
        case 'c': system("CLS"); break;
        }
    }
}
