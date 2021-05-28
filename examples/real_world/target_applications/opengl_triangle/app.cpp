#include "app.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <array>
#include <thread>
using namespace std::chrono_literals;

Application::Application() {
    glfwInit();
    window = {.size_x = 400, .size_y = 400},
    window.glfw_window_ptr = glfwCreateWindow(
        window.size_x, window.size_y,
        "Host OpenGL Application", nullptr, nullptr);

    glfwSetWindowUserPointer(window.glfw_window_ptr, static_cast<void *>(this));

    glfwSetWindowSizeCallback(
        window.glfw_window_ptr,
        [](GLFWwindow *glfw_window_ptr, int size_x, int size_y) {
            auto &app = *static_cast<Application *>(glfwGetWindowUserPointer(glfw_window_ptr));
            app.on_resize(size_x, size_y);
        });

    glfwMakeContextCurrent(window.glfw_window_ptr);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glCreateVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    struct Vertex {
        float x, y;
        float r, g, b;
    };

    std::array vertices{
        // clang-format off
        Vertex{-0.5f, -0.5f, 1.0f, 0.0f, 0.0f},
        Vertex{ 0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
        Vertex{ 0.0f,  0.5f, 0.0f, 0.0f, 1.0f},
        // clang-format on
    };

    glCreateBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, r)));

    shader_program_id = glCreateProgram();
    glUseProgram(shader_program_id);

    const char *const vert_src = R"glsl(
        #version 450

        layout(location = 0) in vec2 in_pos;
        layout(location = 1) in vec3 in_col;

        layout(location = 0) out vec3 out_col;

        void main() {
            out_col = in_col;
            gl_Position = vec4(in_pos, 0, 1);
        }
    )glsl";
    GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader_id, 1, &vert_src, nullptr);
    glCompileShader(vert_shader_id);
    glAttachShader(shader_program_id, vert_shader_id);

    const char *const frag_src = R"glsl(
        #version 450

        layout(location = 0) in vec3 in_col;

        layout(location = 0) out vec4 out_col;

        void main() {
            out_col = vec4(in_col, 1);
        }
    )glsl";
    GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader_id, 1, &frag_src, nullptr);
    glCompileShader(frag_shader_id);
    glAttachShader(shader_program_id, frag_shader_id);

    glLinkProgram(shader_program_id);
    glDeleteShader(vert_shader_id);
    glDeleteShader(frag_shader_id);
}

Application::~Application() {
    glDeleteBuffers(1, &vbo_id);
    glDeleteVertexArrays(1, &vao_id);

    glfwDestroyWindow(window.glfw_window_ptr);
    glfwTerminate();
}

void Application::on_resize(int size_x, int size_y) {
    window.size_x = size_x;
    window.size_y = size_y;

    glViewport(0, 0, size_x, size_y);

    redraw();
}

bool Application::poll_events() {
    glfwPollEvents();
    std::this_thread::sleep_for(4ms);
    return !glfwWindowShouldClose(window.glfw_window_ptr);
}

void Application::redraw() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program_id);
    glBindVertexArray(vao_id);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    std::this_thread::sleep_for(4ms);
    glfwSwapBuffers(window.glfw_window_ptr);
}
