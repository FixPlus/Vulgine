# Vulgine
## General

Vulkan-API based 3D rendering engine. Focused to be easy-to-use along being as flexible as possible.

## Features
### incoming

* uniform / storage buffers support
* multiple programmable render pass support
* multithreading support
* shader modules management
* bunch of post-processing features (ssao, shadow maps, blur etc.)

### enabled

* textured mesh rendering
* chose between dynamic and static vertex/index buffers
* imgui basic features and user input

## Dependencies

Project uses following open-source licensed libraries:
* Vulkan API by Khronos Group: https://www.vulkan.org/
* VMA: https://gpuopen.com/vulkan-memory-allocator/
* GLFW: https://www.glfw.org/
* Dear ImGui: https://github.com/ocornut/imgui
* GLM: https://github.com/g-truc/glm
* stb: https://github.com/nothings/stb

##Platforms

Up to this moment the project has been only built and tested under Windows 10 MinGW environment but is supposed to be supported by GNU/Linux and MacOS in future as well.

## Build
Debug:

    mkdir -p cmake-build-debug && cd cmake-build-debug
    cmake -DCMAKE_BUILD_TYPE=Debug ../
    make -j 8 install

Release:

    mkdir -p cmake-build-release && cd cmake-build-release
    cmake -DCMAKE_BUILD_TYPE=Release ../
    make -j 8 install

## Demo

Project contains executable demo file named *text.exe* built along with the library.