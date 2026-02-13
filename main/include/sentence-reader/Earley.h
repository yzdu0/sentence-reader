#pragma once


#include <string>
#include <vector>
#include <unordered_set>
#include "sentence-reader/Rule.h"
#include "sentence-reader/Grammar.h"
#include "sentence-reader/Lexicon.h"
#include "sentence-reader/SyntaxTree.h"

struct StatePointer {
    std::size_t word_index = 1000;
    std::size_t item_index = 1000;
};

struct WordPointer {
    std::size_t word_index = 1000;
    std::string tag = "-";
    std::string word = "-";
};

struct BackP {
    StatePointer p_left;
    StatePointer p_down;
    WordPointer word_pointer;
};


struct State {
    int rule_id;
    // The dot is BEFORE this index.
    int progress;
    int origin;

    /*StatePointer p_left;
    StatePointer p_down;

    WordPointer word_pointer;*/

    std::vector<BackP> reasons;
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

        for (State& item : items) {
            StateEq eq;
            if (eq(item, s) && s.reasons.size()){
                item.reasons.push_back(s.reasons[0]);
            }
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

    void dfs(StatePointer cur, std::vector<Column>& chart, const std::vector<std::string>& sentence, int depth);

    std::vector<std::string> Earley::dfs3(StatePointer cur,
        std::vector<Column>& chart,
        const std::vector<std::string>& sentence,
        int depth);

    std::vector<std::string> Earley::dfs_helper(StatePointer cur,
        std::vector<Column>& chart,
        const std::vector<std::string>& sentence);

    std::vector<std::string> cartesian_product(const std::vector<std::string>& A,
        const std::vector<std::string>& B);

    std::vector<SyntaxTree> treeDFShelper(StatePointer cur,
        std::vector<Column>& chart,
        const std::vector<std::string>& sentence);

    std::vector<SyntaxTree> treeDFS(StatePointer cur,
        std::vector<Column>& chart,
        const std::vector<std::string>& sentence,
        int depth);

    

    void print_depth(int depth);

    bool is_finished(const State& state) const;

    std::string next_symbol(const State& state) const;

    bool next_symbol_is_nonterminal(const State& state) const;

    void predictor(const State& state, std::size_t k, std::vector<Column>& chart);

    void scanner(const State& state, StatePointer k_and_idx, std::vector<Column>& chart,
        const std::vector<std::string>& sentence);

    void completer (const State& state, StatePointer k_and_idx, std::vector<Column>& chart);

    bool word_has_tag(const std::string& sentence_word, const std::string& symbol) const;
};