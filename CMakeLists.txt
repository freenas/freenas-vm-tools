cmake_minimum_required(VERSION 3.2)
project(freenas_vm_tools)

find_package(Boost 1.59.0 COMPONENTS filesystem program_options)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/main.cc
        src/server.cc
        plugins/system.cc)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(freenas_vm_tools ${SOURCE_FILES})
target_link_libraries(freenas_vm_tools ${Boost_LIBRARIES})
