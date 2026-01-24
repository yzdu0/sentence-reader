cmake_minimum_required(VERSION 3.20)
project(sentence_reader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sentence-reader
    src/main.cpp
    src/Parser.cpp
    src/Rule.cpp
)

target_include_directories(sentence-reader
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
