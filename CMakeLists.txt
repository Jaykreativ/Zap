cmake_minimum_required(VERSION 3.10)
project(Zap)
file(GLOB Zap_SRC
    "src/*"
    "include/*"
)
file(GLOB Zap_Shader_SRC
    "Shader/src/*"
)
file(GLOB PhysX_libs
    "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/PhysX/physx/bin/win.x86_64.vc143.mt/debug/*.lib"
)
add_executable(Zap ${Zap_SRC} ${Zap_Shader_SRC})

source_group("Shader" FILES ${Zap_Shader_SRC})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Zap)
add_subdirectory(Dependencies/Vulkan-Framework)
add_subdirectory(Dependencies/glfw)
add_subdirectory(Dependencies/glm)
target_link_libraries(Zap PUBLIC VulkanFramework glfw glm $ENV{VULKAN_SDK}/Lib/vulkan-1.lib ${PhysX_libs})
target_include_directories(
    Zap PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/Vulkan-Framework/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/glm/glm"
    "${PROJECT_SOURCE_DIR}/Dependencies/PhysX/physx/include"
    "$ENV{VULKAN_SDK}/Include"
)