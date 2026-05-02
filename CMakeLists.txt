cmake_minimum_required(VERSION 3.24)

project(TerrainAndSky)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

if(WIN32)
  target_compile_definitions("${PROJECT_NAME}" PUBLIC "NOMINMAX")
endif()

# Turn off LVK Samples
set(LVK_WITH_SAMPLES OFF CACHE BOOL "" FORCE)
set(LVK_WITH_SAMPLES_ANDROID OFF CACHE BOOL "" FORCE)
set(LVK_DEPLOY_SCREENSHOT_TESTS OFF CACHE BOOL "" FORCE)
# Add LVK sub-directory
add_subdirectory("vendor/lvk")
# Link LVK libraries with target
target_link_libraries(${PROJECT_NAME} PUBLIC LVKLibrary)
target_link_libraries(${PROJECT_NAME} PUBLIC LVKstb)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE SHADERS_DIR="${CMAKE_SOURCE_DIR}/resources/shaders")
target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_DIR="${CMAKE_SOURCE_DIR}/resources")