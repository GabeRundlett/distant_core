#pragma once

struct GLFWwindow;

struct Window {
    int size_x, size_y;
    GLFWwindow *glfw_window_ptr = nullptr;
    constexpr operator bool() noexcept { return static_cast<bool>(glfw_window_ptr); }
};
