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
file(GLOB Zap_Rendering_SRC
	"src/Zap/Rendering/*.cpp"
	"include/Zap/Rendering/*.h"
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

add_library(Zap ${Zap_SRC} ${Zap_Scene_SRC} ${Zap_Shader_SRC} ${Zap_Rendering_SRC} ${Imgui_SRC})

source_group("Source" FILES ${Zap_SRC})
source_group("Source/Scene" FILES ${Zap_Scene_SRC})
source_group("Source/Rendering" FILES ${Zap_Rendering_SRC})
source_group("Shader" FILES ${Zap_Shader_SRC})
source_group("Imgui" FILES ${Imgui_SRC})

set(${BUILD_SHARED_LIBS} false)
set(${CMAKE_BUILD_PARALLEL_LEVEL} 16)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Zap)
set_property(TARGET Zap PROPERTY CXX_STANDARD 17)

add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/VulkanFramework)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/assimp)

set(PHYSX_ROOT_DIR "${PROJECT_SOURCE_DIR}/Dependencies/PhysX/physx")
set(PX_OUTPUT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/PhysX/physx/")
set(PX_OUTPUT_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/PhysX/physx/")
set(TARGET_BUILD_PLATFORM "windows")
set(ENV{PM_PhysXDevice_PATH} "C:/packman-repo/chk/PhysXDevice/18.12.7.6")
set(ENV{PM_freeglut_PATH} "C:/packman-repo/chk/freeglut-windows/3.4_1.1")
set(ENV{PM_PhysXGpu_PATH} "C:/packman-repo/chk/PhysXGpu/912d9405-windows-public")
set(PHYSX_SLN_PHYSXDEVICE_PATH "$ENV{PM_PhysXDevice_PATH}/bin/x86/")
set(PHYSX_SLN_FREEGLUT_PATH "$ENV{PM_freeglut_PATH}/bin/")
set(PHYSX_SLN_PHYSXGPU_PATH "$ENV{PM_PhysXGpu_PATH}/bin/")

#PhysX presets
set(PX_BUILDSNIPPETS OFF)
set(PX_GENERATE_STATIC_LIBRARIES ON)
set(NV_USE_STATIC_WINCRT OFF)
set(NV_USE_DEBUG_WINCRT ON)
set(PX_FLOAT_POINT_PRECISE_MATH OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/PhysX/physx/compiler/public)

target_link_libraries(
	Zap PUBLIC
	VulkanFramework
	glfw
	glm
	assimp
	PhysX
	PhysXCharacterKinematic
	PhysXCommon
	PhysXCooking
	PhysXExtensions
	PhysXVehicle
	PhysXVehicle2
	LowLevel
	LowLevelAABB
	LowLevelDynamics
	SceneQuery
	SimulationController
	PhysXPvdSDK
	PhysXTask
	PhysXFoundation
	$ENV{VULKAN_SDK}/Lib/vulkan-1.lib
)

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