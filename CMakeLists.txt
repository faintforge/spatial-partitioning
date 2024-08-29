cmake_minimum_required(VERSION 3.29)
project(spatial_partitioning)

set(ARKIN_DEBUG ON)
add_subdirectory(libs/arkin)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_executable(${CMAKE_PROJECT_NAME}
    src/main.c
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE include)
target_link_libraries(${CMAKE_PROJECT_NAME} m arkin SDL2)
