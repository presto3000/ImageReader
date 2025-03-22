cmake_minimum_required(VERSION 3.22)

project(
    "ImageProcessing"
    VERSION 1.0.0
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)

option(ENABLE_WARNINGS "Enable to add warnings to a target." ON)
option(ENABLE_WARNINGS_AS_ERRORS "Enable to treat warnings as errors." OFF)
option(ENABLE_CLANG_TIDY "Enable to add clang tidy." ON)
option(ENABLE_CLANG_FORMAT "Enable to add clang-format." ON)

include(Warnings)
#  include(Tools) # needs clang tidy and clang format
include(ConfigSafeGuards)


set (VCPKG_DIR "E:/VCPKG/vcpkg")
include(${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake)

set(OpenCV_DIR "E:/VCPKG/vcpkg/installed/x64-windows/share/opencv4")
set(Protobuf_DIR "E:/VCPKG/vcpkg/installed/x64-windows/share/protobuf")
set(TIFF_DIR "E:/VCPKG/vcpkg/installed/x64-windows/share/tiff")
set(TIFF_LIBRARY "E:/VCPKG/vcpkg/installed/x64-windows/lib/tiff.lib")
set(absl_DIR "E:/VCPKG/vcpkg/installed/x64-windows/share/absl")
set(utf8_range_DIR "E:/VCPKG/vcpkg/installed/x64-windows/share/utf8_range")
set(quirc_DIR "E:/VCPKG/vcpkg/installed/x64-windows/share/quirc")

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(TIFF REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(utf8_range CONFIG REQUIRED)
find_package(quirc CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

add_compile_definitions(NOMINMAX)

target_set_warnings(TARGET ${PROJECT_NAME}
    ENABLE ${ENABLE_WARNINGS}
    AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})
# add_clang_tidy_msvc_to_target(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC src/)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
# Additional Library Directories
link_directories( ${OpenCV_LIB_DIR} )
# Add Protobuf to include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${Protobuf_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${quirc_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE implot::implot)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${TIFF_INCLUDE_DIRS})
# Link Protobuf to your executable
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotobuf)
target_link_libraries(${PROJECT_NAME} PRIVATE ${TIFF_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE utf8_range::utf8_range)
target_link_libraries(${PROJECT_NAME} PRIVATE quirc::quirc)
