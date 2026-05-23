# CMake generated Testfile for 
# Source directory: /Users/lukebartram/Documents/GitHub/sentence-reader/main
# Build directory: /Users/lukebartram/Documents/GitHub/sentence-reader/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[sentence_reader_ambiguous_pp_attachment]=] "/Users/lukebartram/Documents/GitHub/sentence-reader/build/sentence-reader" "--sentence" "I saw the man with the telescope")
set_tests_properties([=[sentence_reader_ambiguous_pp_attachment]=] PROPERTIES  PASS_REGULAR_EXPRESSION "Found 2 interpretation\\(s\\)\\." _BACKTRACE_TRIPLES "/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;30;add_test;/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;0;")
add_test([=[sentence_reader_handles_punctuation]=] "/Users/lukebartram/Documents/GitHub/sentence-reader/build/sentence-reader" "--json" "--sentence" "All humans are mortal, therefore I am not human.")
set_tests_properties([=[sentence_reader_handles_punctuation]=] PROPERTIES  PASS_REGULAR_EXPRESSION "\"success\":true" _BACKTRACE_TRIPLES "/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;38;add_test;/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;0;")
add_test([=[sentence_reader_handles_inflected_verbs]=] "/Users/lukebartram/Documents/GitHub/sentence-reader/build/sentence-reader" "--sentence" "he walks")
set_tests_properties([=[sentence_reader_handles_inflected_verbs]=] PROPERTIES  PASS_REGULAR_EXPRESSION "Found 1 interpretation\\(s\\)\\." _BACKTRACE_TRIPLES "/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;46;add_test;/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;0;")
add_test([=[sentence_reader_json_output]=] "/Users/lukebartram/Documents/GitHub/sentence-reader/build/sentence-reader" "--json" "--sentence" "I saw the man with the telescope")
set_tests_properties([=[sentence_reader_json_output]=] PROPERTIES  PASS_REGULAR_EXPRESSION "\"success\":true" _BACKTRACE_TRIPLES "/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;54;add_test;/Users/lukebartram/Documents/GitHub/sentence-reader/main/CMakeLists.txt;0;")
