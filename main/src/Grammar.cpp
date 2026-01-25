#pragma once
#include <sentence-reader/Grammar.h>
#include <vector>

Grammar::Grammar() {
    std::vector<std::string> rules_string = {
        "S0 -> S",
        "S -> NP VP",
        "S -> S Conj S",

        "VP -> V",
        "VP -> V NP",
        "VP -> VP PP",
        "VP -> V NP PP",
        "VP -> VP Conj VP",
        "VP -> V Inf",
        "VP -> Aux AdjP",
        "VP -> Aux NP",
        "VP -> Aux Neg AdjP",
        "VP -> Aux Neg NP",
        "VP -> Cop AdjP",
        "VP -> Cop NP",

        "NP -> Pron",
        "NP -> Det N",
        "NP -> Det AdjP N",
        "NP -> NP PP",
        "NP -> N",
        "NP -> NP Conj NP",

        "AdjP -> Adj",
        "AdjP -> Adj AdjP",


        "PP -> P NP",
        "PP -> FOR NP Inf",
        "Inf -> TO VP",
        "TO -> to",
        "FOR -> for",

    };

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

// TODO
Grammar Grammar::ChomskyNormalForm(Grammar input) {
    Grammar g;
    return g;
}

void Grammar::enumerate() {

}