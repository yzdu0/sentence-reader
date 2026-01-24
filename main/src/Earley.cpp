#include <string>
#include <vector>
#include "sentence-reader/Rule.h"
#include "sentence-reader/Earley.h"
#include <map>
#include <set>

Earley::Earley() {
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
            "your", "his", "her", "their", "our", "each", "every", "all", "no"
        }},
        {"N", { "man", "woman", "dog", "telescope", "park", "fish", "night", "morning",
            "boy", "girl", "cat", "bird", "city", "car", "house", "tree", "river", "book", "human",
            "teacher", "student", "friend", "child", "food", "music", "movie", "computer", "phone", "humans",
        }},
        {"Adj", { "old", "young", "big", "small", "tall", "short", "fast", "slow",
        "happy", "sad", "angry", "calm", "bright", "dark", "loud", "quiet",
        "new", "good", "bad", "beautiful", "mortal",
        }},
        {"V", {
        "saw", "liked", "walked", "smoked", "ran", "ate", "drank", "slept",
        "talked", "said", "thought", "knew", "found", "made", "took", "gave",
        "looked", "watched", "played", "worked", "tried", "am",
        }},
        {"P", {
        "with", "in", "on", "at", "by", "from", "over", "for",
        "under", "between", "among", "before", "after", "during", "without"
        }},
        {"Conj", {"and", "or", "but", "nor", "yet",
        "so", "therefore"
        }},
        {"TO", {"to"} },
        {"FOR", {"for"}},
        {"Aux", {"am"}},
        {"Neg", {"not", "never"}},
        {"Cop", {"am", "are", "is"}}

    };

    for (std::string rule : rules_string) {
        rules.push_back(Rule(rule));
    }

    for (auto pair : words_map) {
        for (std::string word : pair.second) {
            words.push_back(Rule(pair.first, word));
            //rules.push_back(Rule(pair.first, word));
            word_bank.insert(word);
        }
    }

    for (const Rule& r : rules) {
        nonterminals.insert(r.left);
    }

    for (const Rule& r : words) {
        terminals.insert(r.left);
    }
}

bool Earley::is_finished(const State& state) const {
    if (state.progress >= rules[state.rule_id].right.size()) {
        return true;
    }
    return false;
}

std::string Earley::next_symbol(const State& state) const {
    return rules[state.rule_id].right[state.progress];
}

bool Earley::next_symbol_is_nonterminal(const State& state) const {
    const std::string& sym = next_symbol(state);
    return nonterminals.count(sym) > 0;
}

void Earley::parse(const std::vector<std::string>& sentence,
    const std::vector <std::string>& expected) {
    std::vector<Column> chart(sentence.size() + 1);
    State start_state{ 0, 0, 0 };
    chart[0].add(start_state);

    for (std::size_t k = 0; k <= sentence.size(); k++) {
        // For every state in S(k)
        for (std::size_t x = 0; x < chart[k].size(); x++) {
            const State cur_state = chart[k].items[x]; // COPY it

            if (!is_finished(cur_state)) {
                if (next_symbol_is_nonterminal(cur_state)) {
                    predictor(cur_state, k, chart);
                }
                else {
                    scanner(cur_state, k, chart, sentence);
                }
            }
            else {
                completer(cur_state, k, chart);
            }
        }
    }

    bool accepted = false;
    for (std::size_t i = 0; i < chart.size(); i++) {
        for (const State& s : chart[i].items) {
            if (s.rule_id == 0 && is_finished(s) && s.origin == 0) {
                std::cout << "    Sentence of the first " << i << " words accepted\n    >";

                for (std::size_t j = 0; j < i; j++) std::cout << sentence[j] << " ";
                std::cout << "\n";
            }
        }
    }
}

void Earley::predictor(const State& state, std::size_t k, std::vector<Column>& chart) {
    std::string B = next_symbol(state);

    for (std::size_t i = 0; i < rules.size(); i++) {
        if (rules[i].left == B) {
            State new_state{ i, 0, k };
            chart[k].add(new_state);
        }
    }
}

void Earley::scanner(const State& state, std::size_t k,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence)
{
    if (k >= sentence.size()) return;

    const std::string& tag = next_symbol(state); 

    if (terminals.count(tag) &&
        word_has_tag(sentence[k], tag)) {

        State new_state{ state.rule_id, state.progress + 1, state.origin };

        chart[k + 1].add(new_state);
    }
}


void Earley::completer(const State& state, std::size_t k, std::vector<Column>& chart) {
    const std::string B = rules[state.rule_id].left;

    for (std::size_t i = 0; i < chart[state.origin].size(); i++) {
        const State cur_state = chart[state.origin].items[i];

        if (!is_finished(cur_state) && next_symbol(cur_state) == B) {
            State new_state{ cur_state.rule_id, cur_state.progress + 1, cur_state.origin };
            chart[k].add(new_state);
        }
    }
}

bool Earley::word_has_tag(const std::string& sentence_word, const std::string& symbol) const {
    for (const Rule& rule : words) {
        if (rule.left == symbol && sentence_word == rule.right[0]) {
            return true;
        }
    }
    return false;
}