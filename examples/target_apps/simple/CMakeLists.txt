cmake_minimum_required(VERSION 3.12)
project(${PROJECT_NAME}_Simple)

find_path(CTRE_INCLUDE_DIRS "ctll.hpp")

add_executable(${PROJECT_NAME}
"main.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE DistantCore)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE ${CTRE_INCLUDE_DIRS})
