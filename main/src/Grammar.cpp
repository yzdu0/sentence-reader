#include <sentence-reader/Grammar.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::vector<std::filesystem::path> collect_grammar_files(const std::filesystem::path& root) {
    std::vector<std::filesystem::path> files;

    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::filesystem::path& path = entry.path();
        if (path.extension() != ".txt" || path.filename() == "MakeGrammar.txt") {
            continue;
        }

        files.push_back(path);
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::string> load_grammar_rules() {
    const std::vector<std::filesystem::path> candidates = {
        "language-data/english/grammar",
        "main/language-data/english/grammar",
        "../language-data/english/grammar",
        "../main/language-data/english/grammar",
    };

    for (const std::filesystem::path& candidate : candidates) {
        if (!std::filesystem::exists(candidate) || !std::filesystem::is_directory(candidate)) {
            continue;
        }

        std::vector<std::string> rules;
        for (const std::filesystem::path& path : collect_grammar_files(candidate)) {
            std::ifstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("Could not read grammar file: " + path.string());
            }

            std::string line;
            while (std::getline(file, line)) {
                const std::size_t comment = line.find('#');
                if (comment != std::string::npos) {
                    line.erase(comment);
                }

                line = trim(line);
                if (!line.empty()) {
                    rules.push_back(line);
                }
            }
        }

        if (!rules.empty()) {
            return rules;
        }
    }

    throw std::runtime_error("Could not locate grammar directory under language-data/english/grammar");
}
}

Grammar::Grammar() {
    const std::vector<std::string> rules_string = load_grammar_rules();

    std::vector<std::string> negation_rules_string = {
        "NP VP <-> NP Neg(VP)",
        "S and S <-> Neg(S) or Neg(S)", 
        "S or S <-> Neg(S) and Neg(S)",
        //"S therefore S <-> "
    };
    // I saw the man and I drank a coffee :: I never saw the man or I never drank a coffee
    for (std::string rule : rules_string) {
        rules.push_back(Rule(rule));
    }

    std::vector<std::string> rules_string2 = {
        "S -> NP VP",

        "VP -> V",
        "VP -> V NP",
        "VP -> VP PP",
        "VP -> V NP PP",

        "NP -> Pron",
        "NP -> Det N",
        "NP -> NP PP",
        "NP -> N",

    };

    for (std::string rule : rules_string2) {
        basic_rules.push_back(Rule(rule));
    }
}

Grammar Grammar::ChomskyNormalForm(Grammar input) {
    Grammar g;
    return g;
}

void Grammar::enumerate() {

}
