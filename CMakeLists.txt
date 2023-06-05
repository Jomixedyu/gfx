
cmake_minimum_required (VERSION 3.8)

project ("LearningVulkan1")

set(CMAKE_CXX_STANDARD 23)

include_directories(".")

add_subdirectory(gfx)

aux_source_directory(./ DIR_SRCS)

add_executable (${PROJECT_NAME} ${DIR_SRCS} "shader/lit.hlsl")

target_include_directories(${PROJECT_NAME} PUBLIC gfx)
target_link_libraries(${PROJECT_NAME} gfx)
