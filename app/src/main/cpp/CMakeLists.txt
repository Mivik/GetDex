# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")

cmake_minimum_required(VERSION 3.4.1)

add_library(getdex SHARED getdex.cpp dlopen.c)

add_subdirectory(substrate)

target_include_directories(getdex PRIVATE include)

target_link_libraries(getdex substrate android log)
