cmake_minimum_required(VERSION 3.20)
project(sentence_reader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sentence-reader
    src/Earley.cpp
    src/Grammar.cpp
    src/Lexicon.cpp
    src/Parser.cpp
    src/Rule.cpp
    src/main.cpp
)

target_include_directories(sentence-reader
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_custom_command(TARGET sentence-reader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:sentence-reader>/data
)

option(BUILD_TESTING "Build parser tests" ON)
if(BUILD_TESTING)
    enable_testing()

    add_test(
        NAME sentence_reader_ambiguous_pp_attachment
        COMMAND $<TARGET_FILE:sentence-reader> --sentence "I saw the man with the telescope"
    )
    set_tests_properties(sentence_reader_ambiguous_pp_attachment PROPERTIES
        PASS_REGULAR_EXPRESSION "Found 2 interpretation\\(s\\)\\."
    )

    add_test(
        NAME sentence_reader_handles_punctuation
        COMMAND $<TARGET_FILE:sentence-reader> --sentence "All humans are mortal, therefore I am not human."
    )
    set_tests_properties(sentence_reader_handles_punctuation PROPERTIES
        PASS_REGULAR_EXPRESSION "Found 1 interpretation\\(s\\)\\."
    )

    add_test(
        NAME sentence_reader_handles_inflected_verbs
        COMMAND $<TARGET_FILE:sentence-reader> --sentence "he walks"
    )
    set_tests_properties(sentence_reader_handles_inflected_verbs PROPERTIES
        PASS_REGULAR_EXPRESSION "Found 1 interpretation\\(s\\)\\."
    )
endif()
