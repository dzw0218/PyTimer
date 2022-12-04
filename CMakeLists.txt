cmake_minimum_required(VERSION 3.18)

project(PyTimer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TargetName MyTimer)
add_library(${TargetName} SHARED "")
set_target_properties(${TargetName}
    PROPERTIES
        SUFFIX ".so"
)

add_subdirectory(libs/pybind11)
find_package(Python COMPONENTS Development)

target_include_directories(${TargetName}
    PRIVATE
    ${pybind11_INCLUDE_DIR}
    ${Python_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

file(GLOB_RECURSE src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
target_sources(${TargetName}
    PRIVATE
        ${src}
)

target_link_directories(${TargetName}
    PRIVATE
        ${Python_LIBRARY_DIRS}
)

target_link_libraries(${TargetName}
    PRIVATE
        ${Python_LIBRARIES}
)
