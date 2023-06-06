
cmake_minimum_required (VERSION 3.8)

project ("LearningVulkan1")

set(CMAKE_CXX_STANDARD 23)

include_directories(".")

add_subdirectory(gfx)

aux_source_directory(./ DIR_SRCS)

file(GLOB_RECURSE ImGuiDIR_SRCS "imgui/*.h" "imgui/*.h")

add_executable (${PROJECT_NAME} ${DIR_SRCS} ${ImGuiDIR_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC gfx)
target_link_libraries(${PROJECT_NAME} gfx)

target_link_directories(${PROJECT_NAME} PUBLIC "gfx/GFXThirdParty/glfw")
target_link_libraries(${PROJECT_NAME} glfw3_mt)
target_include_directories(${PROJECT_NAME} PUBLIC imgui)
target_include_directories(${PROJECT_NAME} PUBLIC "gfx/GFXThirdParty/glfw/include")
