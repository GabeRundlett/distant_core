# Distant Core
The core API of the Distant "cheat" framework
Distant is a framework designed to allow developers easily reverse engineer desktop applications. Distant is meant to mimick the functionality, through an API as opposed to strictly a GUI, the functionality of many reverse-engineering tools, including but not limited to Cheat Engine and IDA-pro/Ghidra.

## Building
Distant uses CMake as a build system, meaning you can simply incorporate it directly into your CMake projects.

As for now, the framework itself has **no dependencies**, but in order to build the examples, one would need to have VCPKG set-up (as the CMake toolchain file) with the following libraries installed:
 - glfw3
 - glew
 - imgui[core,glfw-binding,opengl3-glew-binding]

Eventually, the plan is to publish this framework to the VCPKG registry, so that installation and usage is as simple as installing it through the command-line and then adding the single `find_package` line to one's CMake script.

## Rationale
I believe the creation of Distant is necessary because, as far as I know, there is no good reverse-engineering API/Framework. The plan is to use this core API in order to also create a GUI application that supercedes the usability of tools like Cheat Engine and IDA-pro.
