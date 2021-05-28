cmake_minimum_required(VERSION 3.12)
project(${PROJECT_NAME}_Simple)

add_library(${PROJECT_NAME} SHARED 
	"main.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE Distant)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
