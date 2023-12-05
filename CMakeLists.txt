cmake_minimum_required(VERSION 3.10)
project(Zap)
file(GLOB Zap_SRC
    "src/Zap/*.cpp"
    "include/Zap/*.h"
)
file(GLOB Zap_Scene_SRC
    "src/Zap/Scene/*.cpp"
    "include/Zap/Scene/*.h"
)
file(GLOB_RECURSE Zap_Shader_SRC
    "Shader/*"
)
file(GLOB Imgui_SRC
    "Dependencies/imgui/imgui*.cpp"
    "Dependencies/imgui/imgui*.h"
    "Dependencies/imgui/backends/imgui_impl_vulkan.cpp"
    "Dependencies/imgui/backends/imgui_impl_vulkan.h"
    "Dependencies/imgui/backends/imgui_impl_glfw.cpp"
    "Dependencies/imgui/backends/imgui_impl_glfw.h"
)
string(TOLOWER ${CMAKE_BUILD_TYPE} PHYSX_BUILD_TYPE)
file(GLOB PhysX_libs
    "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/PhysX/physx/bin/win.x86_64.vc143.md/${PHYSX_BUILD_TYPE}/*.lib"
)

add_library(Zap ${Zap_SRC} ${Zap_Scene_SRC} ${Zap_Shader_SRC} ${Imgui_SRC})

source_group("Source" FILES ${Zap_SRC})
source_group("Source/Scene" FILES ${Zap_Scene_SRC})
source_group("Shader" FILES ${Zap_Shader_SRC})
source_group("Imgui" FILES ${Imgui_SRC})

set(${BUILD_SHARED_LIBS} false)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Zap)

add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/VulkanFramework)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/assimp)

target_link_libraries(Zap PUBLIC VulkanFramework glfw glm assimp $ENV{VULKAN_SDK}/Lib/vulkan-1.lib ${PhysX_libs})
target_include_directories(
    Zap PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/VulkanFramework/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/imgui"
    "${PROJECT_SOURCE_DIR}/Dependencies/glm/glm"
    "${PROJECT_SOURCE_DIR}/Dependencies/PhysX/physx/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/assimp/include"
    "$ENV{VULKAN_SDK}/Include"
)