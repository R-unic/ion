cmake_minimum_required(VERSION 3.20)
project(Ion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(Ion ${SRC_FILES})

#file(GLOB SPEC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/spec/*.cpp)
#add_executable(IonSpec ${SPEC_FILES})