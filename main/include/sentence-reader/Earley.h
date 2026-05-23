#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>
#include "sentence-reader/Grammar.h"
#include "sentence-reader/Lexicon.h"

struct StatePointer {
    std::size_t word_index = 1000;
    std::size_t item_index = 1000;

    bool operator==(const StatePointer& other) const noexcept {
        return word_index == other.word_index && item_index == other.item_index;
    }
};

struct WordPointer {
    std::size_t word_index = 1000;
    std::string tag = "-";
    std::string word = "-";

    bool operator==(const WordPointer& other) const noexcept {
        return word_index == other.word_index && tag == other.tag && word == other.word;
    }
};

struct BackP {
    StatePointer p_left;
    StatePointer p_down;
    WordPointer word_pointer;

    bool operator==(const BackP& other) const noexcept {
        return p_left == other.p_left
            && p_down == other.p_down
            && word_pointer == other.word_pointer;
    }
};

struct State {
    std::size_t rule_id = 0;
    std::size_t progress = 0;
    std::size_t origin = 0;
    std::vector<BackP> reasons;
};

struct StateHash {
    std::size_t operator()(const State& s) const noexcept {
        std::size_t h = 1469598103934665603ull;
        auto mix = [&](std::size_t x) {
            h ^= x + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
        };
        mix(s.rule_id);
        mix(s.progress);
        mix(s.origin);
        return h;
    }
};

struct StateEq {
    bool operator()(const State& a, const State& b) const noexcept {
        return a.rule_id == b.rule_id
            && a.progress == b.progress
            && a.origin == b.origin;
    }
};

struct ParseResult {
    bool success = false;
    std::vector<std::string> interpretations;
    std::vector<std::string> unknown_words;
};

struct Column {
    std::vector<State> items;
    std::unordered_set<State, StateHash, StateEq> seen;

    bool add(const State& s) {
        if (seen.insert(s).second) {
            items.push_back(s);
            return true;
        }

        if (!s.reasons.empty()) {
            StateEq eq;
            for (State& item : items) {
                if (!eq(item, s)) {
                    continue;
                }
                if (std::find(item.reasons.begin(), item.reasons.end(), s.reasons[0]) == item.reasons.end()) {
                    item.reasons.push_back(s.reasons[0]);
                }
                break;
            }
        }

        return false;
    }

    std::size_t size() const {
        return items.size();
    }
};

class Earley {
public:
    Earley();

    ParseResult parse(const std::vector<std::string>& sentence) const;

private:
    Grammar grammar;
    std::unordered_set<std::string> nonterminals;
    std::unordered_set<std::string> terminals;
    Lexicon lexicon;

    std::vector<std::string> dfs3(StatePointer cur, const std::vector<Column>& chart) const;

    std::vector<std::string> dfs_helper(StatePointer cur, const std::vector<Column>& chart) const;

    std::vector<std::string> cartesian_product(
        const std::vector<std::string>& left,
        const std::vector<std::string>& right) const;

    bool is_finished(const State& state) const;

    const std::string& next_symbol(const State& state) const;

    bool next_symbol_is_nonterminal(const State& state) const;

    void predictor(const State& state, std::size_t k, std::vector<Column>& chart) const;

    void scanner(
        const State& state,
        StatePointer k_and_idx,
        std::vector<Column>& chart,
        const std::vector<std::string>& sentence) const;

    void completer(const State& state, StatePointer k_and_idx, std::vector<Column>& chart) const;

    std::vector<std::string> collect_unknown_words(const std::vector<std::string>& sentence) const;
};
