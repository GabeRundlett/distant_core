#pragma once

#include "window.hpp"

struct Application {
    Window window;
    unsigned int vao_id;
    unsigned int vbo_id;
    unsigned int shader_program_id;

    Application();
    ~Application();
    constexpr operator bool() noexcept { return static_cast<bool>(window); }
    bool poll_events();
    void on_resize(int size_x, int size_y);
    void redraw();
};
