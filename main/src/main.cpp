#include "sentence-reader/Earley.h"
#include <cctype>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::vector<std::string> tokenize_sentence(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;

    for (unsigned char ch : input) {
        if (std::isalnum(ch) || ch == '\'') {
            current.push_back(static_cast<char>(std::tolower(ch)));
            continue;
        }

        if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

std::string join_words(const std::vector<std::string>& words) {
    std::ostringstream out;

    for (std::size_t i = 0; i < words.size(); ++i) {
        if (i > 0) {
            out << ' ';
        }
        out << words[i];
    }

    return out.str();
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [--sentence \"text\"]\n";
}

int parse_and_print(Earley& parser, const std::string& raw_input) {
    const std::vector<std::string> sentence = tokenize_sentence(raw_input);

    if (sentence.empty()) {
        std::cout << "No words found in the input.\n";
        return 1;
    }

    const ParseResult result = parser.parse(sentence);

    std::cout << "Input: " << join_words(sentence) << "\n";

    if (!result.success) {
        std::cout << "No parse found.\n";
        if (!result.unknown_words.empty()) {
            std::cout << "Unknown words: " << join_words(result.unknown_words) << "\n";
        }
        return 1;
    }

    std::cout << "Found " << result.interpretations.size() << " interpretation(s).\n";
    for (std::size_t i = 0; i < result.interpretations.size(); ++i) {
        std::cout << "[" << (i + 1) << "] " << result.interpretations[i] << "\n";
    }

    return 0;
}
}

int main(int argc, char** argv) {
    try {
        Earley parser;
        std::string one_shot_sentence;

        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];

            if (arg == "--help" || arg == "-h") {
                print_usage(argv[0]);
                return 0;
            }

            if (arg == "--sentence" || arg == "-s") {
                if (i + 1 >= argc) {
                    std::cerr << "Missing value for " << arg << ".\n";
                    print_usage(argv[0]);
                    return 2;
                }

                one_shot_sentence = argv[++i];
                continue;
            }

            std::cerr << "Unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return 2;
        }

        if (!one_shot_sentence.empty()) {
            return parse_and_print(parser, one_shot_sentence);
        }

        std::string line;
        while (true) {
            std::cout << "Enter a sentence:\n> ";
            if (!std::getline(std::cin, line)) {
                break;
            }

            if (line.empty()) {
                continue;
            }

            parse_and_print(parser, line);
            std::cout << "\n";
        }

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "Startup error: " << error.what() << "\n";
        return 1;
    }
}
