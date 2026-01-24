#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "sentence-reader/Rule.h"

Rule::Rule(std::string input) {
    enum RuleType {
        Rule,
        Terminal, // classifier of a word
    };
    std::stringstream ss(input);

    std::string item;
    while (getline(ss, item, ' ')) {
        if (left == "") {
            left = item;
        }
        else {
            if (item != "->")
                right.push_back(item);
        }
    }
}

void Rule::print() const {
    std::cout << left << " -> ";
    for (std::string r : right) std::cout << r << " ";
    std::cout << "\n";
}

std::string Rule::lookup_left(std::vector<std::string> right) {
    return "";
}