#pragma once


#include <string>
#include <vector>
#include <unordered_set>
#include "sentence-reader/Rule.h"
#include "sentence-reader/Grammar.h"
#include "sentence-reader/Lexicon.h"

struct StatePointer {
    int word_index = -1;
    int item_index = -1;
};

struct State {
    int rule_id;
    // The dot is BEFORE this index.
    int progress;
    int origin;

    StatePointer previous_state_pointer;
    StatePointer symbol_consumed_pointer;
};

struct StateHash {
    size_t operator()(State const& s) const noexcept {
        size_t h = 1469598103934665603ull;
        auto mix = [&](size_t x) { h ^= x + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2); };
        mix((size_t)s.rule_id);
        mix((size_t)s.progress);
        mix((size_t)s.origin);
        return h;
    }
};
struct StateEq {
    bool operator()(State const& a, State const& b) const noexcept {
        return a.rule_id == b.rule_id && a.progress == b.progress && a.origin == b.origin;
    }
};

struct Column {
    std::vector<State> items;
    std::unordered_set<State, StateHash, StateEq> seen;

    bool add(State s) {
        if (seen.insert(s).second) {
            items.push_back(s);
            return true;
        }
        return false;
    }

    std::size_t size() {
        return items.size();
    }

    void print() {
        for (const State& state : items) {
            std::cout << "   " << state.rule_id << " " << state.progress << " " << state.origin << "\n";
        }
    }
};

class Earley {
public:
	Earley();

	void parse(const std::vector<std::string>& sentence,
		const std::vector<std::string>& expected);

private:
    Grammar grammar;
	std::vector<Rule> rules;
	std::vector<Rule> words;
    std::unordered_set<std::string> nonterminals;
    std::unordered_set<std::string> terminals;
    Lexicon lexicon;

    std::unordered_set<std::string> word_bank;

    void dfs(StatePointer cur, std::vector<Column>& chart);

    bool is_finished(const State& state) const;

    std::string next_symbol(const State& state) const;

    bool next_symbol_is_nonterminal(const State& state) const;

    void predictor(const State& state, std::size_t k, std::vector<Column>& chart);

    void scanner(const State& state, std::size_t k, std::vector<Column>& chart,
        const std::vector<std::string>& sentence);

    void completer (const State& state, StatePointer k_and_idx, std::vector<Column>& chart);

    bool word_has_tag(const std::string& sentence_word, const std::string& symbol) const;
};