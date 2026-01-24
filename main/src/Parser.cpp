#include "sentence-reader/Parser.h"
#include <string>
#include <iostream>
#include <vector>
Parser::Parser() {
    std::vector<std::string> rules_string = {
        "S -> NP VP",
        "VP -> V",
        "VP -> V NP",
        "VP -> VP PP",
        "VP -> V NP PP",

        "NP -> Pron",
        "NP -> Det N",
        "NP -> Det Adj N",
        "NP -> NP PP",
        "NP->N",


        "PP -> P NP",
    };

    std::vector<std::string> words_string = {
        "Pron -> he",
        "Pron -> she",
        "Pron -> they",
        "Pron -> I",
        "Det -> the",
        "Det -> a",
        "Det -> an",
        "N -> man",
        "N -> woman",
        "N -> dog",
        "N -> telescope",
        "N -> park",
        "N -> fish",
        "N -> night",
        "NP -> night",
        "Adj -> old",
        "Adj -> young",
        "Adj -> big",
        "Adj -> small",
        "V -> saw",
        "V -> liked",
        "V -> walked",
        "V -> smoked",
        "P -> with",
        "P -> in",
        "P -> on",
        "P -> at",
    };

    for (std::string rule : rules_string) {
        rules.push_back(Rule(rule));
    }

    for (std::string word : words_string) {
        words.push_back(Rule(word));
    }

    call_depth = -1;
}
bool Parser::parse(
    const std::vector<std::string>& sentence, 
    const std::vector<std::string>& expected) {
    call_depth++;
    /*print_depth();  std::cout << "Call to Parser:\n";
    print_depth();  std::cout << "    Sentence: ";
    for (int i = 0; i < sentence.size(); i++) std::cout << sentence[i] << " ";
    std::cout << "\n";
    print_depth();  std::cout << "    Expecting: ";
    for (int i = 0; i < expected.size(); i++) std::cout << expected[i] << " ";
    std::cout << "\n";*/
    // Base case - 1 word left, 1 expression expected
    if (Completer(sentence, expected)) {
        //print_depth();
        //std::cout << "    Base case reached:\n    > [ " << sentence[0] << " ] [ " << expected[0] << " ]\n";
        call_depth--;

        return true;
    }
    // Recursion case 1 - multiple words left, 1 expression expected
    if (Scanner(sentence, expected)) {
        for (const Rule& rule : rules) {
            if (rule.left == expected[0]) {
                //rule.print();
                //rule.print();
                bool res = parse(sentence, rule.right);
                //std::cout << "HELLO!\n";
                if (res) {
                    //std::cout << "[";  for (std::string word : sentence) std::cout << word << " "; 
                    //std::cout << "] ";
                    //rule.print();

                    //This eliminates branches but test it for now.
                    //std::cout << "Parent: [ "
                    //std::cout << "[ " << expected[0] << " ]\n\n";
                    call_depth--;
                    return true;
                }
            
            }
        }
        call_depth--;
        return false;
    }
    // Recursion case 2 - multiple words left, multiple expressions expected
	std::vector<std::string> cur;
	bool res;
    bool res2 = false;
    // Note that a valid parse here only requires two calls. We parse on the first expression, then another call for the rest of it.
    // Recursion handles everything else. 
	for (std::size_t i = 0; i < sentence.size() - 1; i++) {
		cur.push_back(sentence[i]);
		res = parse(cur, { expected[0] });

		if (res) {
			std::vector<std::string> next_sentence;
			for (std::size_t j = i + 1; j < sentence.size(); j++) {
				next_sentence.push_back(sentence[j]);
			}
            std::vector<std::string> next_expected = expected;
            next_expected.erase(next_expected.begin());
            if (next_sentence.size() && next_expected.size()) {
                res2 = parse(next_sentence, next_expected);
            }
            if (res && res2) {
                std::cout << "Expression Discovered:\n";
                //print_depth();
                std::cout << "> [ ";
                for (std::string word : sentence) std::cout << word << " ";
                std::cout << "] [";
                //print_depth();
                std::cout << lookup_left(expected);
                std::cout << "]~[ ";
                
                for (std::string word : expected) std::cout << word << " ";
                std::cout << "]\n\n";
                call_depth--;
                return true;
            }
		}
	}

    call_depth--;
    return false;
   
}
bool Parser::Predictor(
    const std::vector<std::string>& sentence,
    const std::vector<std::string>& expected) const {

    return false;
}

// Our current sentence is multiple words, but we're only trying to fulfill one word form
bool Parser::Scanner(
    const std::vector<std::string>& sentence,
    const std::vector<std::string>& expected) const {

    if (expected.size() == 1) {
        return true;
    }
    return false;
}

// Our current sentence has only one word 
bool Parser::Completer(
    const std::vector<std::string>& sentence, 
    const std::vector<std::string>& expected) const {

    //std::cout << "HELLO";

    if (sentence.size() == 1) {
        if (expected.size() == 1) {
            //std::cout << sentence[0] << " " << expected[0] << "\n";
            for (const Rule& word : words) {
                //std::cout << word.left << "::" << expected[0] << "--" << word.right[0] << "::" << sentence[0] << "\n";
                if (word.left == expected[0] && word.right[0] == sentence[0]) {
                    return true;
                }
            }
        }
    }
    return false;
}   
void Parser::print_depth() {
    for (int i = 0; i < call_depth * 4; i++) std::cout << " ";
}

std::string Parser::lookup_left(std::vector<std::string> right) const {
    for (const Rule& rule : rules) {
        if (rule.right == right) {
            return rule.left;
        }
    }
    return "";
}