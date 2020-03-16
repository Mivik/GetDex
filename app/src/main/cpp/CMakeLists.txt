# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

set(LIBS_DIR ${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")

cmake_minimum_required(VERSION 3.4.1)

add_library(getdex SHARED getdex.cpp dlopen.c)

add_library(substrate SHARED IMPORTED)
set_target_properties(substrate PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libsubstrate.so)

add_library(substrate_dvm SHARED IMPORTED)
set_target_properties(substrate_dvm PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libsubstrate-dvm.so)

target_include_directories(getdex PRIVATE include)

target_link_libraries(getdex substrate android log)
