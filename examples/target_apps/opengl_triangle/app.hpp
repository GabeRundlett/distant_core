#pragma once

#include "window.hpp"

class Application {
  private:
    Window       window;
    unsigned int vao_id;
    unsigned int vbo_id;
    unsigned int shader_program_id;

  public:
    Application();
    Application(const Application &) = default;
    Application(Application &&)      = delete;
    Application & operator=(const Application &) = default;
    Application & operator=(Application &&) = delete;
    ~Application();
    explicit constexpr operator bool() noexcept {
        return static_cast<bool>(window);
    }
    bool poll_events();
    void on_resize(int size_x, int size_y);
    void redraw();
};
