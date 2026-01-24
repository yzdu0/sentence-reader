#include <iostream>
#include <vector>
#include <map>
#include <ostream>
#include <algorithm>
#include <sstream>
#include <set>
#include "sentence-reader/Parser.h"
#include "sentence-reader/Earley.h"
using namespace std;

void to_lowercase_except(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        });
}

int main() {

    Earley p;

    //std::vector<std::string> sentence = { "the", "man", "smoked", "a", "young", "fish"};
    //std::vector<std::string> sentence = { "I", "saw", "the", "man", "with", "an", "old", "telescope", "in", "the", "park"};
    //std::vector<std::string> sentence = { "they", "saw", "the", "big", "fish", "in", "the", "park", "at", "night", "with", "an", "old", "telescope"};
    
    //std::vector<std::string> sentence = { "they", "saw", "the", "big", "old", "fish", "and", "the", "young", "bird", "at", "night" };
    std::vector<std::string> expected = {"S"};
    
    //p.parse(sentence, expected);

    std::vector<std::string> sentence2 = { "the" };
    std::vector<std::string> expected2 = { "Det" };

    while (1) {
        std::cout << "Enter a sentence:\n >";
        std::string s; 

        std::getline(std::cin, s);

        if (s.empty()) continue;

        std::stringstream ss(s);
        std::string cur;
        std::vector<std::string> sentence;
        while (getline(ss, cur, ' ')) {
            to_lowercase_except(cur);
            sentence.push_back(cur);
        }

        p.parse(sentence, expected);

        std::cout << "\n";
    }

    //p.parse(sentence2, expected2);
    //scan(sentence, st);
}
