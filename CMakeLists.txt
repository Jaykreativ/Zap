cmake_minimum_required(VERSION 3.10)
project(Zap)
add_executable(Zap App.cpp Zap.cpp Window.cpp)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Zap)
add_subdirectory(Dependencies/glfw)
add_subdirectory(Dependencies/Vulkan-Framework)
target_link_libraries(Zap PUBLIC VulkanFramework glfw)
target_include_directories(
    Zap PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/Vulkan-Renderer"
    )