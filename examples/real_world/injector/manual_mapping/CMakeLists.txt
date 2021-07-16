cmake_minimum_required(VERSION 3.12)
project(${PROJECT_NAME}_ManualMapping)

add_executable(${PROJECT_NAME}
    "main.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE DistantCore)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
