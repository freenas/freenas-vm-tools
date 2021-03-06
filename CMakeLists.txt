cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
project(freenas-vm-tools)

set(freenas-vm-tools_VERSION_MAJOR "0")

execute_process(
  COMMAND git rev-list --count master
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE freenas-vm-tools_GIT_COMMIT_COUNT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

find_package(Threads)
find_package(Poco REQUIRED Util Net)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")
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
add_executable(freenas-vm-tools ${MAIN_SOURCE_FILES})
target_link_libraries(freenas-vm-tools ${Poco_LIBRARIES})
target_link_libraries(freenas-vm-tools ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(freenas-vm-tools ${CMAKE_DL_LIBS})
target_link_libraries(freenas-vm-tools -lstdc++)
install (TARGETS freenas-vm-tools DESTINATION sbin)

add_library(system-plugin SHARED ${SYSTEM_SOURCE_FILES})
target_link_libraries(system-plugin ${Poco_LIBRARIES})
target_link_libraries(system-plugin ${CMAKE_THREAD_LIBS_INIT})
install (TARGETS system-plugin DESTINATION lib/freenas-vm-tools)

add_library(file-plugin SHARED ${FILE_SOURCE_FILES})
target_link_libraries(file-plugin ${Poco_LIBRARIES})
target_link_libraries(file-plugin ${CMAKE_THREAD_LIBS_INIT})
install (TARGETS file-plugin DESTINATION lib/freenas-vm-tools)

add_library(network-plugin SHARED ${NETWORK_SOURCE_FILES})
target_link_libraries(network-plugin ${Poco_LIBRARIES})
target_link_libraries(network-plugin ${CMAKE_THREAD_LIBS_INIT})
install (TARGETS network-plugin DESTINATION lib/freenas-vm-tools)

# add_library(time-plugin SHARED ${TIME_SOURCE_FILES})
# target_link_libraries(time-plugin ${Poco_LIBRARIES})
# target_link_libraries(time-plugin ${CMAKE_THREAD_LIBS_INIT})
# install (TARGETS time-plugin DESTINATION lib/freenas-vm-tools)
