#include "sentence-reader/Parser.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>
Parser::Parser() {
    std::vector<std::string> rules_string = {
        "S -> NP VP",
        "S -> S Conj S",

        "VP -> V",
        "VP -> V NP",
        "VP -> VP PP",
        "VP -> V NP PP",
        "VP -> VP Conj VP",
        "VP -> V Inf",

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

	std::map<std::string, std::vector<std::string>> words_map = {
		{"Pron", { "he", "she", "they", "i", "we", "you", "him", "her", "them",
			"me", "us", "it", "who"
		}},
		{"Det", { "the", "a", "an", "this", "that", "these", "those", "my", 
            "your", "his", "her", "their", "our", "each", "every"
		}},
		{"N", { "man", "woman", "dog", "telescope", "park", "fish", "night", "morning",
		    "boy", "girl", "cat", "bird", "city", "car", "house", "tree", "river", "book", 
            "teacher", "student", "friend", "child", "food", "music", "movie", "computer", "phone"
		}},
		{"Adj", { "old", "young", "big", "small", "tall", "short", "fast", "slow",
		"happy", "sad", "angry", "calm", "bright", "dark", "loud", "quiet", 
        "new", "good", "bad", "beautiful"
		}},
		{"V", {
		"saw", "liked", "walked", "smoked", "ran", "ate", "drank", "slept",
		"talked", "said", "thought", "knew", "found", "made", "took", "gave",
		"looked", "watched", "played", "worked", "tried"
		}},
		{"P", {
		"with", "in", "on", "at", "by", "from", "over", "for",
		"under", "between", "among", "before", "after", "during", "without"
		}},
        {"Conj", {"and", "or", "but", "nor", "yet",
        "so",
        }},
        {"TO", {"to"} },
        {"FOR", {"for"}}
	};

    for (std::string rule : rules_string) {
        rules.push_back(Rule(rule));
    }

    for (auto pair : words_map) {
        //words.push_back(Rule(pair.first, pair.second));
        for (std::string word : pair.second) {
            words.push_back(Rule(pair.first, word));
            wordbank.insert(word);
        }
    }

    call_depth = -1;
}
bool Parser::parse(
    const std::vector<std::string>& sentence, 
    const std::vector<std::string>& expected) {
    call_depth++;

    CacheResult r = lookup_cache(sentence, expected);

    if (r == exists_true) return true;
    if (r == exists_false) return false;

    // Base case - 1 word left, 1 expression expected
    if (Completer(sentence, expected)) {
        call_depth--;
        return true;
    }
    // Recursion case 1 - multiple words left, 1 expression expected
    if (Scanner(sentence, expected)) {
        for (const Rule& rule : rules) {
            if (rule.left == expected[0]) {
                bool res = parse(sentence, rule.right);

                add_to_cache(sentence, expected, res); 
                if (res) {
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

        add_to_cache(cur, { expected[0] }, res);

		if (res) {
			std::vector<std::string> next_sentence;
			for (std::size_t j = i + 1; j < sentence.size(); j++) {
				next_sentence.push_back(sentence[j]);
			}
            std::vector<std::string> next_expected = expected;
            next_expected.erase(next_expected.begin());
            if (next_sentence.size() && next_expected.size()) {
                res2 = parse(next_sentence, next_expected);

                add_to_cache(next_sentence, next_expected, res2);
            }
            if (res && res2) {
                //std::cout << "Expression Discovered:\n";
                if (lookup_left(expected) == "S") {
                    std::cout << "Sentence Discovered:\n    ";
                    std::cout << "> [ ";
                    for (std::string word : sentence) std::cout << word << " ";
                    std::cout << "] [";
                    std::cout << lookup_left(expected);
                    std::cout << "]~[ ";
                    for (std::string word : expected) std::cout << word << " ";
                    std::cout << "]\n\n";
                }
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

    if (sentence.size() == 1) {
        if (expected.size() == 1) {
            for (const Rule& word : words) {
                if (word.left == expected[0] && word.right[0] == sentence[0]) {
                    return true;
                }
            }
            for (const Rule& word : rules) {
                if (word.left == expected[0] && word.right[0] == sentence[0]) {
                    return true;
                }
            }

            if (expected[0] == "V" || expected[0] == "Adj" || expected[0] == "N") {
                if (!wordbank.count(sentence[0])) {
                    std::cout << "Guessing... " << sentence[0] << " is a " << expected[0] << "\n";

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

Parser::CacheResult Parser::lookup_cache(
    const std::vector<std::string>& sentence,
    const std::vector<std::string>& expected) const {

    Record key{ sentence, expected };
    auto it = cache.find(key);
    if (it == cache.end()) return CacheResult::nowhere;
    return it->second ? CacheResult::exists_true : CacheResult::exists_false;
}


void Parser::add_to_cache(
    const std::vector<std::string>& sentence,
    const std::vector<std::string>& expected, bool res) {

    Record key{ sentence, expected };
    auto it = cache.find(key);
    if (it == cache.end() && res) {

        std::cout << "Record added:\n    ";
        std::cout << "> [ ";
        for (std::string word : sentence) std::cout << word << " ";
        std::cout << "] [";
        std::cout << lookup_left(expected);
        std::cout << "]~[ ";
        for (std::string word : expected) std::cout << word << " ";
        std::cout << "]\n\n";
    }
    cache[key] = res;
    
}