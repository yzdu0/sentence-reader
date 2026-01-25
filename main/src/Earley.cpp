#include <string>
#include <vector>
#include "sentence-reader/Rule.h"
#include "sentence-reader/Earley.h"
#include "sentence-reader/Lexicon.h"
#include <map>
#include <set>

Earley::Earley() {

    //Lexicon lexicon;
    

    for (auto const& [word_string, word_instance] : lexicon.dictionary) {
        //if (word_instance.POS == "N") {
            //std::cout << word_instance.POS << " <- [" << word_string << "]\n";
       // }
        //words.push_back(Rule(word_instance.POS, word_string));

        for (const Word& word : word_instance) {
            words.push_back(Rule(word.POS, word_string));
        }
    }

    for (const Rule& word : words) {
        word.print();
    }

    for (const Rule& r : grammar.rules) {
        nonterminals.insert(r.left);
    }

    for (const Rule& r : words) {
        terminals.insert(r.left);
    }
}

bool Earley::is_finished(const State& state) const {
    if (state.progress >= grammar.rules[state.rule_id].right.size()) {
        return true;
    }
    return false;
}

std::string Earley::next_symbol(const State& state) const {
    return grammar.rules[state.rule_id].right[state.progress];
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
                    // For the next symbol, we advance to all possible children grammars
                    predictor(cur_state, k, chart);
                }
                else {
                    // The next symbol is a terminal (e.g. just a noun)
                    scanner(cur_state, k, chart, sentence);
                }
            }
            else {
                // We have finished the current state, so we can update all states waiting on the current one
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

    for (std::size_t i = 0; i < grammar.rules.size(); i++) {
        if (grammar.rules[i].left == B) {
            State new_state{ i, 0, k };
            chart[k].add(new_state);
        }
    }
}

void Earley::scanner(const State& state, std::size_t k,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence){

    if (k >= sentence.size()) return;
    const std::string& tag = next_symbol(state); 

    if (terminals.count(tag) &&
        lexicon.search_word(sentence[k], tag)){ 
        //word_has_tag(sentence[k], tag)) {
        State new_state{ state.rule_id, state.progress + 1, state.origin };
        chart[k + 1].add(new_state);
    }
}


void Earley::completer(const State& state, std::size_t k, std::vector<Column>& chart) {
    const std::string B = grammar.rules[state.rule_id].left;

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