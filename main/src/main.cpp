#include "sentence-reader/Earley.h"
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr std::size_t kMaxInterpretationsToOutput = 20;

struct CommandLineOptions {
    std::string sentence;
    bool json_output = false;
};

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

std::string json_escape(const std::string& input) {
    std::ostringstream out;

    for (unsigned char ch : input) {
        switch (ch) {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\b':
            out << "\\b";
            break;
        case '\f':
            out << "\\f";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            if (ch < 0x20) {
                out << "\\u00";
                const char* digits = "0123456789abcdef";
                out << digits[(ch >> 4) & 0x0f] << digits[ch & 0x0f];
            }
            else {
                out << static_cast<char>(ch);
            }
            break;
        }
    }

    return out.str();
}

std::string to_json_array(const std::vector<std::string>& values) {
    std::ostringstream out;
    out << "[";

    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            out << ",";
        }
        out << "\"" << json_escape(values[i]) << "\"";
    }

    out << "]";
    return out.str();
}

std::string build_json_response(
    const std::string& raw_input,
    const std::vector<std::string>& sentence,
    const ParseResult& result,
    const std::string& error_message) {

    std::ostringstream out;
    out << "{";
    out << "\"success\":" << (result.success ? "true" : "false") << ",";
    out << "\"rawInput\":\"" << json_escape(raw_input) << "\",";
    out << "\"normalizedInput\":\"" << json_escape(join_words(sentence)) << "\",";
    out << "\"tokens\":" << to_json_array(sentence) << ",";
    out << "\"interpretationCount\":" << result.interpretations.size() << ",";
    out << "\"interpretations\":" << to_json_array(result.interpretations) << ",";
    out << "\"unknownWords\":" << to_json_array(result.unknown_words) << ",";
    out << "\"error\":\"" << json_escape(error_message) << "\"";
    out << "}";
    return out.str();
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [--json] [--sentence \"text\"]\n";
}

ParseResult limit_interpretations_for_output(const ParseResult& result) {
    ParseResult limited = result;
    if (limited.interpretations.size() > kMaxInterpretationsToOutput) {
        limited.interpretations.resize(kMaxInterpretationsToOutput);
    }
    return limited;
}

int parse_and_print(Earley& parser, const std::string& raw_input, bool json_output) {
    const std::vector<std::string> sentence = tokenize_sentence(raw_input);
    ParseResult empty_result;

    if (sentence.empty()) {
        const std::string error_message = "No words found in the input.";
        if (json_output) {
            std::cout << build_json_response(raw_input, sentence, empty_result, error_message) << "\n";
            return 0;
        }

        std::cout << error_message << "\n";
        return 1;
    }

    const ParseResult result = parser.parse(sentence);
    const ParseResult limited_result = limit_interpretations_for_output(result);
    const std::string error_message = result.success ? "" : "No parse found.";

    if (json_output) {
        std::cout << build_json_response(raw_input, sentence, limited_result, error_message) << "\n";
        return 0;
    }

    std::cout << "Input: " << join_words(sentence) << "\n";

    if (!result.success) {
        std::cout << "No parse found.\n";
        if (!result.unknown_words.empty()) {
            std::cout << "Unknown words: " << join_words(result.unknown_words) << "\n";
        }
        return 1;
    }

    std::cout << "Found " << result.interpretations.size() << " interpretation(s).\n";
    if (result.interpretations.size() > limited_result.interpretations.size()) {
        std::cout << "Showing the first " << limited_result.interpretations.size() << ".\n";
    }
    for (std::size_t i = 0; i < limited_result.interpretations.size(); ++i) {
        std::cout << "[" << (i + 1) << "] " << limited_result.interpretations[i] << "\n";
    }

    return 0;
}

CommandLineOptions parse_arguments(int argc, char** argv) {
    CommandLineOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            std::exit(0);
        }

        if (arg == "--json") {
            options.json_output = true;
            continue;
        }

        if (arg == "--sentence" || arg == "-s") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Missing value for " + arg + ".");
            }

            options.sentence = argv[++i];
            continue;
        }

        throw std::runtime_error("Unknown argument: " + arg);
    }

    return options;
}
}

int main(int argc, char** argv) {
    bool json_output = false;

    try {
        const CommandLineOptions options = parse_arguments(argc, argv);
        json_output = options.json_output;

        Earley parser;

        if (!options.sentence.empty()) {
            return parse_and_print(parser, options.sentence, json_output);
        }

        if (json_output) {
            ParseResult empty_result;
            std::cout << build_json_response("", {}, empty_result, "JSON mode requires --sentence.") << "\n";
            return 2;
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

            parse_and_print(parser, line, false);
            std::cout << "\n";
        }

        return 0;
    }
    catch (const std::exception& error) {
        if (json_output) {
            ParseResult empty_result;
            std::cout << build_json_response("", {}, empty_result, error.what()) << "\n";
            return 1;
        }

        std::cerr << "Startup error: " << error.what() << "\n";
        return 1;
    }
}
