#include <string>
#include <vector>
#include "sentence-reader/Rule.h"
#include "sentence-reader/Earley.h"
#include "sentence-reader/Lexicon.h"
#include "sentence-reader/Util.h"
#include "sentence-reader/SyntaxTree.h"
#include <map>
#include <set>

Earley::Earley() {

    for (auto const& [word_string, word_instance] : lexicon.dictionary) {
        for (const Word& word : word_instance) {
            words.push_back(Rule(word.POS, word_string));
        }
    }
    /*for (const Rule& word : words) {
        word.print();
        std::cout << "\n";
    }*/

    for (const Rule& r : grammar.rules) 
        nonterminals.insert(r.left);

    for (const Rule& r : words) 
        terminals.insert(r.left);
}

void Earley::dfs(StatePointer cur,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence,
    int depth){

    // Only for invalid sentences
    //if (depth > 1) return;

    //if (cur.word_index >= 1000 || cur.item_index >= 1000) return;

    const State& s = chart[cur.word_index].items[cur.item_index];

    if (is_finished(s) && depth >= 0) {
        print_depth(depth);

        std::cout << "[" << sentence[s.origin];
        for (std::size_t i = s.origin + 1; i < cur.word_index; i++) {
            std::cout << " " << sentence[i];
        }
        std::cout << "] [";
        grammar.rules[s.rule_id].print();
        std::cout << "] ";
        if (s.reasons.size() > 1) std::cout << "[Ambiguous]";
        std::cout << "\n";
        //<< s.reasons.size() << "\n";
    }

    if (s.reasons.size() == 0) return;

    dfs(s.reasons[0].p_left, chart, sentence, depth);

    // Recurse only if this was from completion
    if (s.reasons[0].p_down.word_index < 1000 && s.reasons[0].p_down.item_index < 1000) {
        dfs(s.reasons[0].p_down, chart, sentence, depth + 1);
    }
    // Otherwise print the leaf (from scan)
    else if (s.reasons[0].word_pointer.word_index < 1000) {
        print_depth(depth + 1);
        std::cout << "[" << s.reasons[0].word_pointer.word << "] ["
            << s.reasons[0].word_pointer.tag << " -> " << s.reasons[0].word_pointer.word
            << "]\n";
    }
}
static bool valid(const StatePointer& p) {
    return p.word_index < 1000 && p.item_index < 1000;
}
static bool has_down(const BackP& bp) { return valid(bp.p_down); }

static bool has_word(const BackP& bp) { return bp.word_pointer.word_index < 1000; }

std::vector<std::string> Earley::cartesian_product(const std::vector<std::string>& A,
    const std::vector<std::string>& B) {
    std::vector<std::string> out;
    for (auto& a : A) {
        for (auto& b : B) {
            out.push_back(a + b);
        }
    }
    return out;
}

// WORST function to code EVER :v:
// Given a state, returns the right hand of the grammar rule eg the NP VP in S -> NP VP 
std::vector<std::string> Earley::dfs_helper(StatePointer cur,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence)
{
    // base cases 
    //if (!valid(cur)) return { "" };
    const State& st = chart[cur.word_index].items[cur.item_index];

    if (st.progress == 0) return { "" };

    std::vector<std::string> all;
    // Loop through every possible path to this state 
    for (const BackP& bp : st.reasons) {

        std::vector<std::string> rest = dfs_helper(bp.p_left, chart, sentence);

        std::vector<std::string> child_alts;
        // Recursive descent; to another state
        if (has_down(bp)) {
            child_alts = dfs3(bp.p_down, chart, sentence, 0);
        }
        // Base case: we reach a single word
        else if (has_word(bp)) {
            child_alts = { "(" + bp.word_pointer.tag + " \"" + bp.word_pointer.word + "\")" };
        }
        else {
            child_alts = { "" };
        }
        for (auto& c : child_alts) c = " " + c;

        std::vector<std::string> combined = Util::cartesian_product<std::string>(rest, child_alts);
        for (auto& x : combined) {
            all.push_back(x);
        }
    }
    return all;
}

std::vector<std::string> Earley::dfs3(StatePointer cur,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence,
    int depth)
{
    if (!valid(cur)) return { "" };

    const State& s = chart[cur.word_index].items[cur.item_index];
    if (!is_finished(s)) return { "" };

    std::string lhs = grammar.rules[s.rule_id].left;

    // Build RHS alternatives (children sequence)
    std::vector<std::string> rhs = dfs_helper(cur, chart, sentence);

    std::vector<std::string> out;
    for (auto& r : rhs) {
        out.push_back("(" + lhs + r + ")");
    }
    return out;
}

/*
I saw the man

VP -> Det V

This is represented by three states.
VP -> * Det V
VP -> Det * V
VP -> Det V *


VP   VP   VP
|    |    |
|    |    V
|    progress
starting
*/
std::vector<SyntaxTree> Earley::treeDFShelper(StatePointer cur,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence) {
    if (!valid(cur)) return {};
    // base cases 
    const State& st = chart[cur.word_index].items[cur.item_index];

    if (st.progress == 0) return { SyntaxTree(grammar.rules[st.rule_id].left) };

    std::vector<SyntaxTree> all;

    // Loop through every possible path to this state 
    //BackP bp = st.reasons[0];
    for (const BackP& bp : st.reasons) {
        std::vector<SyntaxTree> rest = treeDFShelper(bp.p_left, chart, sentence);

        std::vector<SyntaxTree> child;
        // Recursive descent; to another state
        if (has_down(bp)) {
            child = treeDFShelper(bp.p_down, chart, sentence);
        }
        // Base case: we reach a single word
        else if (has_word(bp)) {
            child = { SyntaxTree(bp.word_pointer.tag, bp.word_pointer.word) };
        }
        else {

        }

        std::vector<SyntaxTree> combined = Util::cartesian_product(rest, child);
        for (SyntaxTree& s : combined) {
            all.push_back(s);
        }
    }
    //rest.addTreeRight(child);
    return all;

    //return rest + child;
}

// (S0(S(NP(Pron "i"))(VP(V "saw")(NP(NP(Det "the")(N "man"))(PP(P "with")(NP(Det "my")(N "telescope")))))))
// (S0(S(NP(Pron "i"))(VP(VP(V "saw")(NP(Det "the")(N "man")))(PP(P "with")(NP(Det "my")(N "telescope"))))))
// 
// 
// ( (S (NP (Pron "i")) (VP (VP (V "saw") (NP (Det "the") (N "man"))) (PP (P "with") (NP (Det "a") (N "telescope"))))))
// ( (S (NP (Pron "i")) (VP (V "saw") (NP (NP (Det "the") (N "man")) (PP (P "with") (NP (Det "the") (N "telescope"))))))) 
void Earley::print_depth(int depth) {
    for (int i = 0; i < depth; i++) std::cout << "|   ";
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

        for (std::size_t x = 0; x < chart[k].size(); x++) {
            const State cur_state = chart[k].items[x]; // COPY it

            if (!is_finished(cur_state)) {
                if (next_symbol_is_nonterminal(cur_state)) {
                    // For the next symbol, we advance to all possible children grammars
                    predictor(cur_state, k, chart);
                }
                else {
                    // The next symbol is a terminal (e.g. just a noun)
                    StatePointer k_and_idx{ k, x };
                    scanner(cur_state, k_and_idx, chart, sentence);
                }
            }
            else {
                // We have finished the current state, so we can update all states waiting on the current one
                StatePointer k_and_idx{ k, x };
                completer(cur_state, k_and_idx, chart);
            }
        }
    }

    //std::cout << "Syntax Tree:\n";
    StatePointer res;
    for (std::size_t i = 0; i < chart.back().size(); i++) {
        const State& s = chart.back().items[i];
        if (s.rule_id == 0 && is_finished(s) && s.origin == 0) {
            res.word_index = chart.size() - 1;
            res.item_index = i;
        }
    }
    //dfs(res, chart, sentence, -1);

    std::vector<std::string> x = dfs3(res, chart, sentence, -1);

    if (x.size() == 1) {
        std::cout << "1 interpretation discovered:\n";
    }
    else {
        std::cout << x.size() << " interpretations discovered:\n";
    }

    //for (std::string cur : x) {
    //    std::cout << cur << "\n";
    //}

    std::vector<SyntaxTree> syntaxTrees = treeDFShelper(res, chart, sentence);
    //syntaxTrees[0].display();
    std::cout << "\n";
    for (SyntaxTree& s : syntaxTrees) {
        s.display();
        std::cout << "----\n";
    }
}
// Generate new states
void Earley::predictor(const State& state, std::size_t k, std::vector<Column>& chart) {
    std::string B = next_symbol(state);

    for (std::size_t i = 0; i < grammar.rules.size(); i++) {
        if (grammar.rules[i].left == B) {
            State new_state{ i, 0, k };
            chart[k].add(new_state);
        }
    }
}
/*
This one updates a state when the next symbol is a terminal (e.g. a verb), and we
can check directly if it is satisfied. 
Therefore, the node corresponding to the new state should have a pointer to the raw symbol/word. 
*/
void Earley::scanner(const State& state, StatePointer k_and_idx,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence){

    if (k_and_idx.word_index >= sentence.size()) return;
    const std::string& tag = next_symbol(state); 

    if (terminals.count(tag) && lexicon.search_word(sentence[k_and_idx.word_index], tag)){ 
        State new_state{ state.rule_id, state.progress + 1, state.origin};

        std::string word_ = sentence[k_and_idx.word_index];
        WordPointer word_pointer{ k_and_idx.word_index, tag, word_};

        new_state.reasons.push_back({ k_and_idx, StatePointer{}, word_pointer });
        chart[k_and_idx.word_index + 1].add(new_state);
    }
}

// Updates all states waiting on the current state, now that the current one is completed
void Earley::completer(const State& state, StatePointer k_and_idx, std::vector<Column>& chart) {
    const std::string B = grammar.rules[state.rule_id].left;

    for (std::size_t i = 0; i < chart[state.origin].size(); i++) {
        const State cur_state = chart[state.origin].items[i];

        /*
        cur_state is the previous state that we have incremented. This will be what previous_state_pointer points to.
        state (the argument) is the state that was consumed. 
        */
        if (!is_finished(cur_state) && next_symbol(cur_state) == B) {
            StatePointer previous_state_pointer{ state.origin, i };

            State new_state{ cur_state.rule_id, cur_state.progress + 1, cur_state.origin};
            new_state.reasons.push_back({ previous_state_pointer, k_and_idx, WordPointer{} });
            chart[k_and_idx.word_index].add(new_state);
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