cmake_minimum_required(VERSION 3.12)
project(${PROJECT_NAME}_Core)

list(APPEND ${PROJECT_NAME}_sources
    "core.cpp"
)

if (WIN32)
    list(APPEND ${PROJECT_NAME}_sources
        "platform/windows/injection.cpp"
        "platform/windows/process.cpp"
        "platform/windows/virtual_buffer.cpp"
        "platform/windows/module.cpp"
    )
endif()

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
