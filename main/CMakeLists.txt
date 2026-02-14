cmake_minimum_required(VERSION 3.20)
project(sentence_reader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sentence-reader
    src/main.cpp
    src/deprecated/Parser.cpp
    src/Rule.cpp
    src/Earley.cpp
    src/Lexicon.cpp
    src/SyntaxTree.cpp "src/Grammar.cpp" "include/sentence-reader/Grammar.h" "include/sentence-reader/SyntaxTree.h" "include/sentence-reader/Util.h")

target_include_directories(sentence-reader
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_custom_command(TARGET sentence-reader POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/data
          $<TARGET_FILE_DIR:sentence-reader>/data)