#include "sentence-reader/Earley.h"
#include <algorithm>

namespace {
constexpr const char* kStartSymbol = "S0";

bool valid(const StatePointer& p) {
    return p.word_index < 1000 && p.item_index < 1000;
}

bool has_down(const BackP& bp) {
    return valid(bp.p_down);
}

bool has_word(const BackP& bp) {
    return bp.word_pointer.word_index < 1000;
}
}

Earley::Earley() {
    for (const Rule& rule : grammar.rules) {
        nonterminals.insert(rule.left);
    }

    for (const auto& [surface_form, analyses] : lexicon.dictionary) {
        (void)surface_form;
        for (const Word& analysis : analyses) {
            terminals.insert(analysis.POS);
        }
    }
}

std::vector<std::string> Earley::cartesian_product(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right) const {

    std::vector<std::string> out;
    out.reserve(left.size() * right.size());

    for (const std::string& lhs : left) {
        for (const std::string& rhs : right) {
            out.push_back(lhs + rhs);
        }
    }

    return out;
}

std::vector<std::string> Earley::dfs_helper(
    StatePointer cur,
    const std::vector<Column>& chart) const {

    if (!valid(cur)) {
        return { "" };
    }

    const State& state = chart[cur.word_index].items[cur.item_index];
    if (state.progress == 0) {
        return { "" };
    }

    std::vector<std::string> all;

    for (const BackP& reason : state.reasons) {
        const std::vector<std::string> rest = dfs_helper(reason.p_left, chart);

        std::vector<std::string> child_alternatives;
        if (has_down(reason)) {
            child_alternatives = dfs3(reason.p_down, chart);
        }
        else if (has_word(reason)) {
            child_alternatives = {
                "(" + reason.word_pointer.tag + " \"" + reason.word_pointer.word + "\")"
            };
        }
        else {
            child_alternatives = { "" };
        }

        for (std::string& child : child_alternatives) {
            if (!child.empty()) {
                child = " " + child;
            }
        }

        const std::vector<std::string> combined = cartesian_product(rest, child_alternatives);
        all.insert(all.end(), combined.begin(), combined.end());
    }

    return all;
}

std::vector<std::string> Earley::dfs3(
    StatePointer cur,
    const std::vector<Column>& chart) const {

    if (!valid(cur)) {
        return { "" };
    }

    const State& state = chart[cur.word_index].items[cur.item_index];
    if (!is_finished(state)) {
        return { "" };
    }

    const std::string& lhs = grammar.rules[state.rule_id].left;
    const std::vector<std::string> rhs = dfs_helper(cur, chart);

    std::vector<std::string> out;
    out.reserve(rhs.size());

    for (const std::string& expansion : rhs) {
        out.push_back("(" + lhs + expansion + ")");
    }

    return out;
}

bool Earley::is_finished(const State& state) const {
    return state.progress >= grammar.rules[state.rule_id].right.size();
}

const std::string& Earley::next_symbol(const State& state) const {
    return grammar.rules[state.rule_id].right[state.progress];
}

bool Earley::next_symbol_is_nonterminal(const State& state) const {
    return nonterminals.count(next_symbol(state)) > 0;
}

ParseResult Earley::parse(const std::vector<std::string>& sentence) const {
    ParseResult result;
    result.unknown_words = collect_unknown_words(sentence);

    if (sentence.empty()) {
        return result;
    }

    std::vector<Column> chart(sentence.size() + 1);
    for (std::size_t i = 0; i < grammar.rules.size(); ++i) {
        if (grammar.rules[i].left == kStartSymbol) {
            chart[0].add(State{ i, 0, 0 });
        }
    }

    if (chart[0].size() == 0) {
        return result;
    }

    for (std::size_t k = 0; k <= sentence.size(); ++k) {
        for (std::size_t x = 0; x < chart[k].size(); ++x) {
            const State current = chart[k].items[x];

            if (!is_finished(current)) {
                if (next_symbol_is_nonterminal(current)) {
                    predictor(current, k, chart);
                }
                else {
                    scanner(current, StatePointer{ k, x }, chart, sentence);
                }
            }
            else {
                completer(current, StatePointer{ k, x }, chart);
            }
        }
    }

    StatePointer completed_sentence;
    bool found_complete_parse = false;
    for (std::size_t i = 0; i < chart.back().size(); ++i) {
        const State& state = chart.back().items[i];
        if (grammar.rules[state.rule_id].left == kStartSymbol
            && state.origin == 0
            && is_finished(state)) {
            completed_sentence = StatePointer{ chart.size() - 1, i };
            found_complete_parse = true;
            break;
        }
    }

    if (!found_complete_parse) {
        return result;
    }

    result.success = true;
    result.interpretations = dfs3(completed_sentence, chart);

    std::sort(result.interpretations.begin(), result.interpretations.end());
    result.interpretations.erase(
        std::unique(result.interpretations.begin(), result.interpretations.end()),
        result.interpretations.end());

    return result;
}

void Earley::predictor(const State& state, std::size_t k, std::vector<Column>& chart) const {
    const std::string& symbol = next_symbol(state);

    for (std::size_t i = 0; i < grammar.rules.size(); ++i) {
        if (grammar.rules[i].left == symbol) {
            chart[k].add(State{ i, 0, k });
        }
    }
}

void Earley::scanner(
    const State& state,
    StatePointer k_and_idx,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence) const {

    if (k_and_idx.word_index >= sentence.size()) {
        return;
    }

    const std::string& tag = next_symbol(state);
    const std::string& word = sentence[k_and_idx.word_index];

    if (terminals.count(tag) == 0 || !lexicon.search_word(word, tag)) {
        return;
    }

    State advanced_state{ state.rule_id, state.progress + 1, state.origin };
    advanced_state.reasons.push_back({
        k_and_idx,
        StatePointer{},
        WordPointer{ k_and_idx.word_index, tag, word }
    });

    chart[k_and_idx.word_index + 1].add(advanced_state);
}

void Earley::completer(const State& state, StatePointer k_and_idx, std::vector<Column>& chart) const {
    const std::string& completed_symbol = grammar.rules[state.rule_id].left;

    for (std::size_t i = 0; i < chart[state.origin].size(); ++i) {
        const State waiting_state = chart[state.origin].items[i];
        if (is_finished(waiting_state) || next_symbol(waiting_state) != completed_symbol) {
            continue;
        }

        State advanced_state{ waiting_state.rule_id, waiting_state.progress + 1, waiting_state.origin };
        advanced_state.reasons.push_back({
            StatePointer{ state.origin, i },
            k_and_idx,
            WordPointer{}
        });

        chart[k_and_idx.word_index].add(advanced_state);
    }
}

std::vector<std::string> Earley::collect_unknown_words(const std::vector<std::string>& sentence) const {
    std::vector<std::string> unknown_words;

    for (const std::string& word : sentence) {
        if (!lexicon.contains_word(word)) {
            unknown_words.push_back(word);
        }
    }

    std::sort(unknown_words.begin(), unknown_words.end());
    unknown_words.erase(std::unique(unknown_words.begin(), unknown_words.end()), unknown_words.end());

    return unknown_words;
}
