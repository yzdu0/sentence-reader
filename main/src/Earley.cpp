#include <string>
#include <vector>
#include "sentence-reader/Rule.h"
#include "sentence-reader/Earley.h"
#include "sentence-reader/Lexicon.h"
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

//void Earley::dfs(StatePointer cur, std::vector<Column>& chart, const std::vector<std::string>& sentence, int depth) {
//    if (cur.word_index < 1000 && cur.item_index < 1000) {
//		const State& s = chart[cur.word_index].items[cur.item_index];
//
//        if (is_finished(s)) {
//            print_depth(depth);
//
//            if (depth >= 0) {
//                std::cout << "[" << sentence[s.origin];
//                for (std::size_t i = s.origin + 1; i < cur.word_index; i++) {
//                    std::cout << " " << sentence[i];
//                }
//                std::cout << "] [";
//
//                grammar.rules[s.rule_id].print();
//                std::cout << "]\n";
//
//            }
//        }
//        dfs(s.p_left, chart, sentence, depth);
//        dfs(s.p_down, chart, sentence, depth + 1);
//	}
//}

void Earley::dfs(StatePointer cur,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence,
    int depth){

    if (cur.word_index >= 1000 || cur.item_index >= 1000) return;

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

    //dfs(s.reasons[0].p_left, chart, sentence, depth);
}

static bool valid(const StatePointer& p) {
    return p.word_index < 1000 && p.item_index < 1000;
}

static bool has_down(const BackP& bp) {
    return valid(bp.p_down);
}

static bool has_word(const BackP& bp) {
    return bp.word_pointer.word_index < 1000; // assuming default 1000 means null
}

std::string Earley::dfs2(StatePointer cur,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence,
    int depth)
{
    if (!valid(cur)) return "";

    const State& s = chart[cur.word_index].items[cur.item_index];

    if (!is_finished(s)) return "";

    const BackP& reason = s.reasons[0];

    std::vector<std::string> children;
    StatePointer walk = cur;

    while (true) {
        const State& ws = chart[walk.word_index].items[walk.item_index];
        if (ws.reasons.empty()) break;

        const BackP& bp = ws.reasons[0];

        if (has_down(bp)) {
            children.push_back(dfs2(bp.p_down, chart, sentence, depth + 1));
        }
        else if (has_word(bp)) {
            children.push_back("(" + bp.word_pointer.tag + " \"" + bp.word_pointer.word + "\")");
        }

        if (!valid(bp.p_left)) break;
        walk = bp.p_left;
    }

    std::reverse(children.begin(), children.end());

    std::string out = "(";

    if (depth >= 0) out += grammar.rules[s.rule_id].left; // skip S0

    for (const auto& c : children) {
        if (!c.empty()) out += " " + c;
    }
    out += ")";

    return out;
}

std::vector<std::string> Earley::dfs3(StatePointer cur,
    std::vector<Column>& chart,
    const std::vector<std::string>& sentence,
    int depth)
{
    if (!valid(cur)) return { "" };

    const State& s = chart[cur.word_index].items[cur.item_index];

    if (!is_finished(s)) return { "" };

    //const BackP& reason = s.reasons[0];
    std::vector<std::string> res;
    std::vector<std::string> children;
    StatePointer walk = cur;
    while (true) {
        const State& ws = chart[walk.word_index].items[walk.item_index];
        if (ws.reasons.empty()) break;
        const BackP& bp = ws.reasons.back();
        if (has_down(bp)) {
            children.push_back(dfs3(bp.p_down, chart, sentence, depth + 1)[0]);
        }
        else if (has_word(bp)) {
            children.push_back("(" + bp.word_pointer.tag + " \"" + bp.word_pointer.word + "\")");
        }
        if (!valid(bp.p_left)) break;
        walk = bp.p_left;
    }
    std::reverse(children.begin(), children.end());
    std::string out = "(";
    if (depth >= 0) out += grammar.rules[s.rule_id].left; // skip S0
    for (const auto& c : children) {
        if (!c.empty()) out += " " + c;
    }
    out += ")";
    res.push_back(out);

    return res;
}
// ( (S (NP (Pron "i")) (VP (VP (V "saw") (NP (Det "the") (N "man"))) (PP (P "with") (NP (Det "a") (N "telescope"))))))  
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

    std::cout << "Syntax Tree:\n";
    StatePointer res;
    for (std::size_t i = 0; i < chart.back().size(); i++) {
        const State& s = chart.back().items[i];
        if (s.rule_id == 0 && is_finished(s) && s.origin == 0) {
            res.word_index = chart.size() - 1;
            res.item_index = i;
        }
    }
    
    std::vector<std::string> x = dfs3(res, chart, sentence, -1);

    for (std::string cur : x) {
        std::cout << cur << "\n";
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
        //new_state.p_left = k_and_idx;
        //new_state.p_down = StatePointer{};

        std::string word_ = sentence[k_and_idx.word_index];
        WordPointer word_pointer{ k_and_idx.word_index, tag, word_};
        //new_state.word_pointer = word_pointer;

        new_state.reasons.push_back({ k_and_idx, StatePointer{}, word_pointer });
        chart[k_and_idx.word_index + 1].add(new_state);
    }
}

/*
Because the argument state has reached its end, we are trying to propagate its status to
all of the previous states directly relying on this one. 
For example, the super simple sentence "I saw".
After "saw", we manage to resolve a verb phrase (VP -> V, starting at index 1). 
If we go back to look at index 1, there is another state (S -> NP dot VP starting at index 0)
that is relying on this verb phrase. 

On its own, Completer will create an updated state at index 2; this state is an exact copy but 
with the progress incremented by one (e.g. S -> NP VP). 
We can see during this update that the new S relies on VP. 
Therefore, we want the node corresponding to S to have another pointer to a child; in this case the VP state. 

(Also note that for an ambiguous sentence, there would be multiple ways this state can be reached, but only 
one is used in the Chart so we don't solve the same problem twice/go into a recursive infinite loop.)


---

For each State, we store two pointers. A pointer that points to the previous version of the state
e.g. (S->NP VP dot, 0) in chart[2] has a pointer to (S->NP dot VP, 0) in chart[1]. 
And also a pointer that points to the symbol that was consumed in order to advance the state.
In this case, we're pointing to (VP -> V dot, 1) in chart[2]. 

Of course, the (S->NP dot VP, 0) state in chart[1] will also have two pointers. One to the previous version
in chart[0], and another to the symbol that was consumed to advance to this one; (NP -> Pron) in chart[1].

By backtracking along the "state evolution" pointers from the originally mentioned (S->NP VP dot, 0) state,
we encounter all of the (S-> NP VP) states used to evolve this. Each of these contains another pointer,
the pointer being used to consume.
In fact the direction doesn't actually matter, since the pointers to "consumed symbols" may as well have
interesting parse trees of their own. So we can just DFS from the completed sentence node, and this will show
us the entire forest of interest. 

Later on, I need to ensure each state can store an array of (pairs of) pointers, so we can parse for ambuigity.
Scanner needs to own pointers too but the idea will be very similar.
*/
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

            //new_state.p_left = previous_state_pointer;
            //new_state.p_down = k_and_idx;

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