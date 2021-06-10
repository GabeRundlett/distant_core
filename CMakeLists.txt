cmake_minimum_required(VERSION 3.12)
project(Distant)

add_subdirectory(src)

option(DISTANT_BUILD_EXAMPLES "Build distant examples" ON)
if (${DISTANT_BUILD_EXAMPLES})
	add_subdirectory(examples)
endif()
