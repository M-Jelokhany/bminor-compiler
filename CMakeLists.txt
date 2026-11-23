
cmake_minimum_required(VERSION 3.16)

project(BMinorCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(BMinorCompiler
    main.cpp
    src/SourceCursor.cpp
    src/Lexer.cpp
    src/AST.cpp
    src/Parser.cpp
)

target_include_directories(BMinorCompiler PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
