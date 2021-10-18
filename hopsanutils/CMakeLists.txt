cmake_minimum_required(VERSION 3.1)
project(HopsanCoreUtils)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()


set(target_name hopsanutils)

# Create library target and add source code files
add_library(${target_name} STATIC)
# Include source code
include(${CMAKE_CURRENT_LIST_DIR}/files.cmake)

# Set include directories
target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                                    $<INSTALL_INTERFACE:include>)

# Set link dependencies
target_link_libraries(${target_name} PUBLIC hopsancore)
