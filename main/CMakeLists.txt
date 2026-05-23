cmake_minimum_required(VERSION 3.20)
project(sentence_reader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sentence-reader
    src/Earley.cpp
    src/Grammar.cpp
    src/Lexicon.cpp
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
        COMMAND $<TARGET_FILE:sentence-reader> --json --sentence "All humans are mortal, therefore I am not human."
    )
    set_tests_properties(sentence_reader_handles_punctuation PROPERTIES
        PASS_REGULAR_EXPRESSION "\"success\":true"
    )

    add_test(
        NAME sentence_reader_handles_inflected_verbs
        COMMAND $<TARGET_FILE:sentence-reader> --sentence "he walks"
    )
    set_tests_properties(sentence_reader_handles_inflected_verbs PROPERTIES
        PASS_REGULAR_EXPRESSION "Found 1 interpretation\\(s\\)\\."
    )

    add_test(
        NAME sentence_reader_json_output
        COMMAND $<TARGET_FILE:sentence-reader> --json --sentence "I saw the man with the telescope"
    )
    set_tests_properties(sentence_reader_json_output PROPERTIES
        PASS_REGULAR_EXPRESSION "\"success\":true"
    )

    add_test(
        NAME sentence_reader_handles_aux_adv_number_np
        COMMAND $<TARGET_FILE:sentence-reader> --json --sentence "Alice could quickly write three careful letters."
    )
    set_tests_properties(sentence_reader_handles_aux_adv_number_np PROPERTIES
        PASS_REGULAR_EXPRESSION "\"success\":true"
    )

    add_test(
        NAME sentence_reader_handles_relative_clause
        COMMAND $<TARGET_FILE:sentence-reader> --json --sentence "The scientist who discovered the planet smiled."
    )
    set_tests_properties(sentence_reader_handles_relative_clause PROPERTIES
        PASS_REGULAR_EXPRESSION "\"success\":true"
    )

    add_test(
        NAME sentence_reader_handles_fronted_pp_and_cop
        COMMAND $<TARGET_FILE:sentence-reader> --json --sentence "In the morning the teacher remained calm."
    )
    set_tests_properties(sentence_reader_handles_fronted_pp_and_cop PROPERTIES
        PASS_REGULAR_EXPRESSION "\"success\":true"
    )

    add_test(
        NAME sentence_reader_handles_inflected_adjectives
        COMMAND $<TARGET_FILE:sentence-reader> --json --sentence "The happier child is very quiet."
    )
    set_tests_properties(sentence_reader_handles_inflected_adjectives PROPERTIES
        PASS_REGULAR_EXPRESSION "\"success\":true"
    )
endif()
