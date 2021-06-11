#include <cstdlib>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

struct Application {
    GLFWwindow *glfw_window_ptr;
    ImFont *arial_font;
};

void dockspace() {
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpaceWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();
}

void on_draw(int size_x, int size_y) {
    static float rgb[3];
    static bool show_demo = false;

    // dockspace();

    ImGui::Begin("MainUI", nullptr, ImGuiWindowFlags_NoDecoration);

    if (ImGui::Button("Open Process"))
        ImGui::OpenPopup("Open Process");
    if (ImGui::BeginPopupModal("Open Process")) {
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::ColorEdit3("Clear Color", rgb);
    ImGui::Checkbox("Show Demo Window", &show_demo);

    ImGui::End();

    ImGui::Begin("AddressList");
    ImGui::End();

    if (show_demo)
        ImGui::ShowDemoWindow(&show_demo);

    glClearColor(rgb[0], rgb[1], rgb[2], 1.0f);
}

void redraw(GLFWwindow *glfw_window_ptr, int size_x, int size_y) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    on_draw(size_x, size_y);

    ImGui::Render();
    glViewport(0, 0, size_x, size_y);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(glfw_window_ptr);
}

Application app_init() {
    glfwInit();
    Application result;
    result.glfw_window_ptr = glfwCreateWindow(800, 600, "App window", nullptr, nullptr);

    glfwMakeContextCurrent(result.glfw_window_ptr);
    glfwSetWindowSizeCallback(result.glfw_window_ptr, [](GLFWwindow *glfw_window_ptr, int size_x, int size_y) {
        redraw(glfw_window_ptr, size_x, size_y);
    });

    glewInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = NULL;

    ImGui::StyleColorsDark();
    result.arial_font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf", 14.0f);

    ImGui_ImplGlfw_InitForOpenGL(result.glfw_window_ptr, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    return result;
}

int main() {
    auto app = app_init();

    while (true) {
        glfwPollEvents();
        if (glfwWindowShouldClose(app.glfw_window_ptr))
            break;
        int display_w, display_h;
        glfwGetFramebufferSize(app.glfw_window_ptr, &display_w, &display_h);
        redraw(app.glfw_window_ptr, display_w, display_h);
    }

    return EXIT_SUCCESS;
}
