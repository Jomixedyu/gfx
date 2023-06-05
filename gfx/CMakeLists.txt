
project ("gfx")

file(GLOB_RECURSE gfx_SRC "include/*.h" "src/*.h" "src/*.cpp")

add_library(${PROJECT_NAME} STATIC OBJECT ${gfx_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "include" "src/PlatformImpl/Vulkan")

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC "GFXThirdParty")
target_link_directories(${PROJECT_NAME} PUBLIC "GFXThirdParty/glfw")
target_link_libraries(${PROJECT_NAME} PUBLIC glfw3_mt)
