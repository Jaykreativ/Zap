cmake_minimum_required(VERSION 3.10)
project(Zap)
file(GLOB Zap_SRC
    "*.cpp"
    "*.h"
)
add_executable(Zap ${Zap_SRC})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Zap)
add_subdirectory(Dependencies/Vulkan-Framework)
add_subdirectory(Dependencies/glfw)
add_subdirectory(Dependencies/glm)
target_link_libraries(Zap PUBLIC VulkanFramework glfw glm $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
target_include_directories(
    Zap PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/Dependencies/Vulkan-Framework"
    "${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/glm/glm"
    "$ENV{VULKAN_SDK}/Include"
)