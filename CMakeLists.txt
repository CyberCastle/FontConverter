# Script based in this: https://gist.github.com/WesThorburn/00c47b267a0e8c8431e06b14997778e4
cmake_minimum_required(VERSION 3.5.1)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(BUILD_SHARED_LIBS OFF)

project (fontconverter C CXX)

#add_definitions(-O3 -Wall)
add_definitions(-std=c++17 -Wall -g4)
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/vendor/freetype2/include)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/lib/*.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")
add_executable(fontconverter ${SOURCES})

message(STATUS "Setting compilation target to WASM")
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# Flags to build with emscripten are described here: 
# https://github.com/emscripten-core/emscripten/blob/incoming/src/settings.js
set_target_properties(fontconverter PROPERTIES LINK_FLAGS "-s WASM=1 -s MODULARIZE=1 -s NODERAWFS=1 -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS='[_main, _convert, _convertcb]' -s EXTRA_EXPORTED_RUNTIME_METHODS='[cwrap, addFunction, removeFunction, AsciiToString]' -s STRICT=1 -s INCOMING_MODULE_JS_API='[arguments, noInitialRun]' -s ENVIRONMENT=node -s ALLOW_TABLE_GROWTH=1")
target_link_libraries(fontconverter ${CMAKE_SOURCE_DIR}/vendor/freetype2/build/libfreetype.a)
