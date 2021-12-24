cmake_minimum_required(VERSION 3.12)
project(${PROJECT_NAME}_App)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(${PROJECT_NAME} 
    "main.cpp"
    "imgui_impl_glfw.cpp"
    "imgui_impl_opengl3.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad::glad imgui::imgui fmt::fmt)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_options(${PROJECT_NAME} PRIVATE 
# $<$<PLATFORM_ID:Windows>:
#     "/ENTRY:mainCRTStartup"
#     "/SUBSYSTEM:WINDOWS"
# >
)
