cmake_minimum_required(VERSION 4.4)

project(BMinorCompiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(BMinorCompiler
        main.cpp
        src/SourceCursor.cpp
        include/bminor/SourceLocation.h
        include/bminor/Token.h
        include/bminor/SourceCursor.h
)

target_include_directories(BMinorCompiler PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
