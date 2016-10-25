cmake_minimum_required(VERSION 3.2)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
project(freenas_vm_tools)

set(freenas_vm_tools_VERSION_MAJOR 0)
set(freenas_vm_tools_VERSION_MINOR 1)

find_package(Threads)
find_package(Poco REQUIRED Util Net)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-register")

set(MAIN_SOURCE_FILES
        unix/Main.cc
        src/Server.cc
        src/Context.cc
        unix/UnixDevice.cc)

set(SYSTEM_SOURCE_FILES plugins/System.cc)
set(NETWORK_SOURCE_FILES plugins/Network.cc)
set(FILE_SOURCE_FILES plugins/File.cc src/Utils.cc)
set(TIME_SOURCE_FILES plugins/Time.cc)

configure_file(
        src/Config.hh.in
        Config.hh)

include_directories(${Poco_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} include/ contrib/)
add_executable(freenas_vm_tools ${MAIN_SOURCE_FILES})
target_link_libraries(freenas_vm_tools ${Poco_LIBRARIES})
target_link_libraries(freenas_vm_tools ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(freenas_vm_tools ${CMAKE_DL_LIBS})

add_library(system_plugin SHARED ${SYSTEM_SOURCE_FILES})
target_link_libraries(system_plugin ${Poco_LIBRARIES})
target_link_libraries(system_plugin ${CMAKE_THREAD_LIBS_INIT})

add_library(file_plugin SHARED ${FILE_SOURCE_FILES})
target_link_libraries(file_plugin ${Poco_LIBRARIES})
target_link_libraries(file_plugin ${CMAKE_THREAD_LIBS_INIT})

add_library(network_plugin SHARED ${NETWORK_SOURCE_FILES})
target_link_libraries(network_plugin ${Poco_LIBRARIES})
target_link_libraries(network_plugin ${CMAKE_THREAD_LIBS_INIT})

add_library(time_plugin SHARED ${TIME_SOURCE_FILES})
target_link_libraries(time_plugin ${Poco_LIBRARIES})
target_link_libraries(time_plugin ${CMAKE_THREAD_LIBS_INIT})
