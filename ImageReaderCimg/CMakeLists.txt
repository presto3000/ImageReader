cmake_minimum_required(VERSION 3.22)

# Set vcpkg toolchain file
set(VCPKG_DIR "E:/VCPKG/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Project name
project(ImageReader VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(OpenCV REQUIRED)

# Add the executable
add_executable(${PROJECT_NAME} ImageProgram.cpp)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Link OpenCV libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

# Enable warnings
option(ENABLE_WARNINGS "Enable to add warnings to a target." ON)
option(ENABLE_WARNINGS_AS_ERRORS "Enable to treat warnings as errors." OFF)

if(ENABLE_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
    )
endif()

if(ENABLE_WARNINGS_AS_ERRORS)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<CXX_COMPILER_ID:GNU>:-Werror>
        $<$<CXX_COMPILER_ID:Clang>:-Werror>
    )
endif()
