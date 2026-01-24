#include <iostream>
#include <vector>
#include <map>
#include <ostream>
#include <algorithm>
#include <sstream>
#include <set>
#include "sentence-reader/Parser.h"
using namespace std;
/*
S  -> NP VP

VP -> V
VP -> V NP
VP -> VP PP
VP -> V NP PP

NP -> Pron
NP -> Det N
NP -> Det Adj N
NP -> NP PP
NP -> N

PP -> P NP
Pron -> "he" | "she" | "they"
Det  -> "the" | "a"
N    -> "man" | "woman" | "dog" | "telescope" | "park"
Adj  -> "old" | "young" | "big" | "small"
V    -> "saw" | "liked" | "walked"
P    -> "with" | "in" | "on"
*/

int main() {

    Parser p;

    //std::vector<std::string> sentence = { "the", "man", "smoked", "a", "young", "fish"};
    //std::vector<std::string> sentence = { "I", "saw", "the", "man", "with", "an", "old", "telescope", "in", "the", "park"};
    std::vector<std::string> sentence = { "they", "saw", "the", "fish", "at", "night", "with", "an", "old", "telescope" };
    std::vector<std::string> expected = { "S" };
    
    p.parse(sentence, expected);

    std::vector<std::string> sentence2 = { "the" };
    std::vector<std::string> expected2 = { "Det" };

    //p.parse(sentence2, expected2);
    //scan(sentence, st);
}
