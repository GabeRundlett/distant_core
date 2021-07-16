#include <iostream>
#include <string>
#include <format>
#include <thread>
using namespace std::chrono_literals;

#include <ctre.hpp>

struct ApplicationInfo {
    int  value;
    bool should_quit : 1, should_change : 1;
};

auto expressions_range(const std::string & str) {
    // /(?<exp>[\w][^;]*)/gm
    return ctre::range<R"reg((?<exp>[\w][^;]*))reg">(str);
}

void process_expression(const std::string & exp) {
    std::cout << std::format(" exp: {}\n", exp);
    // /([A-Za-z0-9]*)/gm
    if (auto match = ctre::match<R"reg(([A-Za-z0-9]*))reg">(exp)) {
        std::cout << std::format("  - {}\n", std::string_view{match.get<0>()});
    }
}

int main() {
    static ApplicationInfo app_static{
        .value         = 4,
        .should_quit   = false,
        .should_change = true,
    };
    ApplicationInfo app_stack   = app_static;
    static auto     app_dynamic = std::make_unique<ApplicationInfo>(app_static);

    std::cout << "enter q to quit, enter h for help\n";

    while (!app_static.should_quit) {
        std::string user_input;
        getline(std::cin, user_input);
        std::cout << std::format("[inputted '{}']\n", user_input);
        for (auto match : expressions_range(user_input)) {
            const auto exp = std::string_view{match.get<"exp">()};
            process_expression(std::string(exp));
        }
    }
}
