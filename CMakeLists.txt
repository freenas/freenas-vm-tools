cmake_minimum_required(VERSION 3.2)
project(freenas_vm_tools)

add_definitions(-DBOOST_LOG_DYN_LINK)

find_package(Threads)
find_package(Boost COMPONENTS system thread filesystem program_options log log_setup)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-register")

set(MAIN_SOURCE_FILES
        unix/main.cc
        src/server.cc
        src/context.cc
        src/utils.cc
        unix/unix_device.cc
        unix/unix_loader.cc)

set(SYSTEM_SOURCE_FILES
        plugins/system.cc)

set(FILE_SOURCE_FILES
        plugins/file.cc
        src/utils.cc)

include_directories(${Boost_INCLUDE_DIRS} include/)
add_executable(freenas_vm_tools ${MAIN_SOURCE_FILES})
target_link_libraries(freenas_vm_tools ${Boost_LIBRARIES})
target_link_libraries(freenas_vm_tools ${CMAKE_THREAD_LIBS_INIT})

add_library(system_plugin SHARED ${SYSTEM_SOURCE_FILES})
target_link_libraries(system_plugin ${Boost_LIBRARIES})
target_link_libraries(system_plugin ${CMAKE_THREAD_LIBS_INIT})

add_library(file_plugin SHARED ${FILE_SOURCE_FILES})
target_link_libraries(file_plugin ${Boost_LIBRARIES})
target_link_libraries(file_plugin ${CMAKE_THREAD_LIBS_INIT})
