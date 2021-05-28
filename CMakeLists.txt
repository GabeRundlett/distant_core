cmake_minimum_required(VERSION 3.12)
project(Distant)

list(APPEND ${PROJECT_NAME}_sources
    "src/core.cpp"
)

if (WIN32)
    list(APPEND ${PROJECT_NAME}_sources
        "src/platform/windows/injection.cpp"
        "src/platform/windows/process.cpp"
        "src/platform/windows/virtual_buffer.cpp"
        "src/platform/windows/module.cpp"
    )
endif()

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

option(DISTANT_BUILD_EXAMPLES "Build distant examples" ON)
if (${DISTANT_BUILD_EXAMPLES})
	add_subdirectory(examples)
endif()
