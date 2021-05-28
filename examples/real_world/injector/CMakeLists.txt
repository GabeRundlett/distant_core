cmake_minimum_required(VERSION 3.12)
project(${PROJECT_NAME}_Injector)

add_subdirectory(load_library)
add_subdirectory(manual_mapping)

add_executable(${PROJECT_NAME}
    "main.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE Distant)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
