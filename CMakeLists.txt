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
	"Shader/src/*"
)
file(GLOB Imgui_SRC
	"Dependencies/imgui/imgui*.cpp"
	"Dependencies/imgui/imgui*.h"
	"Dependencies/imgui/backends/imgui_impl_vulkan.cpp"
	"Dependencies/imgui/backends/imgui_impl_vulkan.h"
	"Dependencies/imgui/backends/imgui_impl_glfw.cpp"
	"Dependencies/imgui/backends/imgui_impl_glfw.h"
)

source_group("Source" FILES ${Zap_SRC})
source_group("Source/Scene" FILES ${Zap_Scene_SRC})
source_group("Source/Rendering" FILES ${Zap_Rendering_SRC})
source_group("Shader" FILES ${Zap_Shader_SRC})
source_group("Imgui" FILES ${Imgui_SRC})

set(${BUILD_SHARED_LIBS} false)
set(${CMAKE_BUILD_PARALLEL_LEVEL} 16)

add_library(Zap ${Zap_SRC} ${Zap_Scene_SRC} ${Zap_Shader_SRC} ${Zap_Rendering_SRC} ${Imgui_SRC})

set_property(TARGET Zap PROPERTY CXX_STANDARD 17)

#Search Vulkan
find_package(Vulkan REQUIRED)
IF (NOT Vulkan_FOUND)
	find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${PROJECT_SOURCE_DIR}/lib/vulkan)
	set(Vulkan_INLUDE_DIRS "${PROJECT_SOURCE_DIR}/Dependencies/vulkan/include")
	set(Vulkan_GLSLC_EXECUTABLE "${PROJECT_SOURCE_DIR}/bin/vulkan/glslc.exe")
	set(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE "${PROJECT_SOURCE_DIR}/bin/vulkan/glslangValidator.exe")
	IF (Vulkan_LIBRARY)
		set(Vulkan_FOUND ON)
		MESSAGE("Using bundled Vulkan library version")
	ENDIF()
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

add_compile_definitions(GLSLANG_VALIDATOR="${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}")
add_compile_definitions(GLSLC="${Vulkan_GLSLC_EXECUTABLE}")

#foreach(SHADER IN LISTS Zap_Shader_SRC) TODO try to add custom shader compile command using cmake
#	get_filename_component(FILENAME ${SHADER} NAME)
#	set(SPIRV_OUTPUT "${CMAKE_SOURCE_DIR}/${FILENAME}.spv")
#
#	message(STATUS "add glslc.exe compile command for ${FILENAME}")
#
#	add_custom_command(
#		OUTPUT ${SPIRV_OUTPUT}
#		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${CMAKE_SOURCE_DIR}/${FILENAME}.spv
#		DEPENDS ${SHADER}
#		COMMENT "Compiling ${FILENAME} to ${SPIRV_OUTPUT}"
#		VERBATIM
#	)
#
#	list(APPEND SPIRV_SHADERS ${SPIRV_SHADERS})
#endForeach()
#
#add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})
#add_dependencies(Zap Shaders)

add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/VulkanFramework)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dependencies/assimp)

set(PHYSX_ROOT_DIR "${PROJECT_SOURCE_DIR}/Dependencies/PhysX/physx")
set(PX_OUTPUT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/PhysX/physx/")
set(PX_OUTPUT_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/PhysX/physx/")
set(TARGET_BUILD_PLATFORM "windows")
set(PHYSX_SLN_PHYSXDEVICE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/physx")
set(PHYSX_SLN_FREEGLUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/physx")

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
	${Vulkan_LIBRARY}
)

target_include_directories(
	Zap PUBLIC
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/Dependencies/VulkanFramework/include"
	"${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
	"${PROJECT_SOURCE_DIR}/Dependencies/imgui"
	"${PROJECT_SOURCE_DIR}/Dependencies/glm/glm"
	"${PROJECT_SOURCE_DIR}/Dependencies/PhysX/physx/include"
	"${PROJECT_SOURCE_DIR}/Dependencies/assimp/include"
	${Vulkan_INLUDE_DIRS}
)