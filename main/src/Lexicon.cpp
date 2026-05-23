#include "sentence-reader/Lexicon.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

namespace {
bool is_vowel(char ch) {
    switch (ch) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
        return true;
    default:
        return false;
    }
}

bool ends_with(const std::string& word, const std::string& suffix) {
    return word.size() >= suffix.size()
        && word.compare(word.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool ends_with_any(const std::string& word, const std::vector<std::string>& suffixes) {
    for (const std::string& suffix : suffixes) {
        if (ends_with(word, suffix)) {
            return true;
        }
    }
    return false;
}

std::string build_regular_plural(const std::string& lemma) {
    if (ends_with_any(lemma, { "s", "sh", "ch", "x", "z" })) {
        return lemma + "es";
    }
    if (lemma.size() > 1 && lemma.back() == 'y'
        && !ends_with_any(lemma, { "ay", "ey", "iy", "oy", "uy" })) {
        return lemma.substr(0, lemma.size() - 1) + "ies";
    }
    return lemma + "s";
}

bool ends_with_cvc(const std::string& word) {
    if (word.size() < 3 || word.size() > 4) {
        return false;
    }

    if (ends_with_any(word, { "en", "er", "el" })) {
        return false;
    }

    const char first = word[word.size() - 3];
    const char second = word[word.size() - 2];
    const char third = word[word.size() - 1];

    return !is_vowel(first)
        && is_vowel(second)
        && !is_vowel(third)
        && third != 'w'
        && third != 'x'
        && third != 'y';
}

std::string build_regular_past(const std::string& lemma) {
    if (lemma.size() > 1 && lemma.back() == 'y' && !is_vowel(lemma[lemma.size() - 2])) {
        return lemma.substr(0, lemma.size() - 1) + "ied";
    }
    if (ends_with(lemma, "e")) {
        return lemma + "d";
    }
    if (ends_with_cvc(lemma)) {
        return lemma + lemma.back() + "ed";
    }
    return lemma + "ed";
}

std::string build_third_person_singular(const std::string& lemma) {
    if (ends_with_any(lemma, { "s", "sh", "ch", "x", "z", "o" })) {
        return lemma + "es";
    }
    if (lemma.size() > 1 && lemma.back() == 'y'
        && !ends_with_any(lemma, { "ay", "ey", "iy", "oy", "uy" })) {
        return lemma.substr(0, lemma.size() - 1) + "ies";
    }
    return lemma + "s";
}

std::string build_gerund(const std::string& lemma) {
    if (ends_with(lemma, "ie")) {
        return lemma.substr(0, lemma.size() - 2) + "ying";
    }
    if (lemma.size() > 1 && lemma.back() == 'e' && !ends_with(lemma, "ee")) {
        return lemma.substr(0, lemma.size() - 1) + "ing";
    }
    if (ends_with_cvc(lemma)) {
        return lemma + lemma.back() + "ing";
    }
    return lemma + "ing";
}

bool prefers_periphrastic_comparison(const std::string& lemma) {
    return ends_with_any(lemma, {
        "ful", "ous", "ive", "less", "ing", "ed", "able", "ible", "al",
        "ic", "ish", "ent", "ant", "ary", "ory"
    });
}

std::string build_regular_comparative(const std::string& lemma) {
    if (lemma.size() > 1 && lemma.back() == 'y' && !is_vowel(lemma[lemma.size() - 2])) {
        return lemma.substr(0, lemma.size() - 1) + "ier";
    }
    if (ends_with(lemma, "e")) {
        return lemma + "r";
    }
    if (ends_with_cvc(lemma)) {
        return lemma + lemma.back() + "er";
    }
    return lemma + "er";
}

std::string build_regular_superlative(const std::string& lemma) {
    if (lemma.size() > 1 && lemma.back() == 'y' && !is_vowel(lemma[lemma.size() - 2])) {
        return lemma.substr(0, lemma.size() - 1) + "iest";
    }
    if (ends_with(lemma, "e")) {
        return lemma + "st";
    }
    if (ends_with_cvc(lemma)) {
        return lemma + lemma.back() + "est";
    }
    return lemma + "est";
}

std::ifstream open_vocab_file() {
    const std::vector<std::string> candidates = {
        "data/vocab.txt",
        "main/data/vocab.txt",
        "../data/vocab.txt",
        "../main/data/vocab.txt"
    };

    for (const std::string& candidate : candidates) {
        std::ifstream file(candidate);
        if (file.is_open()) {
            return file;
        }
    }

    return {};
}
}

Word::Word() : word(""), POS("") {
}

Word::Word(std::string word, std::string POS)
    : word(std::move(word)), POS(std::move(POS)) {

    if (!this->POS.empty() && this->POS.back() == ':') {
        this->POS.pop_back();
    }
}

Lexicon::Lexicon() {
    std::ifstream file = open_vocab_file();
    if (!file.is_open()) {
        throw std::runtime_error("Could not locate vocab.txt");
    }

    std::string line_text;

    while (std::getline(file, line_text)) {
        if (line_text.empty() || line_text[0] == '#') {
            continue;
        }

        std::stringstream line_stream(line_text);
        std::string token;
        std::vector<std::string> line;

        while (std::getline(line_stream, token, ' ')) {
            if (!token.empty() && token != "|") {
                line.push_back(token);
            }
        }

        if (line.empty()) {
            continue;
        }

        if (line[0] == "N:") {
            create_nouns(line);
        }
        else if (line[0] == "V:") {
            create_verbs(line);
        }
        else if (line[0] == "Adj:") {
            create_adjectives(line);
        }
        else {
            create_misc(line);
        }
    }
}

void Lexicon::add_to_dictionary(const std::string& word, const std::string& POS) {
    auto [it, inserted] = dictionary.emplace(word, std::vector<Word>{});
    (void)inserted;

    for (const Word& entry : it->second) {
        if (entry.word == word && entry.POS == POS) {
            return;
        }
    }

    it->second.push_back(Word(word, POS));
}

bool Lexicon::search_word(const std::string& word, const std::string& POS) const {
    auto it = dictionary.find(word);
    if (it == dictionary.end()) {
        return false;
    }

    for (const Word& entry : it->second) {
        if (entry.word == word && entry.POS == POS) {
            return true;
        }
    }

    return false;
}

bool Lexicon::contains_word(const std::string& word) const {
    return dictionary.find(word) != dictionary.end();
}

void Lexicon::create_nouns(const std::vector<std::string>& input) {
    const std::string& base_word = input[1];
    const std::string& POS = input[0];

    add_to_dictionary(base_word, POS);

    std::string plural_version = find_overload(input, "plural");
    if (plural_version == "-") {
        plural_version = build_regular_plural(base_word);
    }

    add_to_dictionary(plural_version, POS);
}

void Lexicon::create_verbs(const std::vector<std::string>& input) {
    const std::string& POS = input[0];
    const std::string& base_word = input[1];

    add_to_dictionary(base_word, POS);

    std::string past_tense = find_overload(input, "past");
    if (past_tense == "-") {
        past_tense = build_regular_past(base_word);
    }
    add_to_dictionary(past_tense, POS);

    std::string third_person_singular = find_overload(input, "pres3");
    if (third_person_singular == "-") {
        third_person_singular = build_third_person_singular(base_word);
    }
    add_to_dictionary(third_person_singular, POS);

    std::string gerund = find_overload(input, "gerund");
    if (gerund == "-") {
        gerund = build_gerund(base_word);
    }
    add_to_dictionary(gerund, POS);

    std::string participle = find_overload(input, "part");
    if (participle == "-") {
        participle = past_tense;
    }
    add_to_dictionary(participle, POS);
}

void Lexicon::create_adjectives(const std::vector<std::string>& input) {
    const std::string& base_word = input[1];
    const std::string& POS = input[0];

    add_to_dictionary(base_word, POS);

    std::string comparative = find_overload(input, "comparative");
    if (comparative != "-") {
        add_to_dictionary(comparative, POS);
    }
    else if (!prefers_periphrastic_comparison(base_word)) {
        add_to_dictionary(build_regular_comparative(base_word), POS);
    }

    std::string superlative = find_overload(input, "superlative");
    if (superlative != "-") {
        add_to_dictionary(superlative, POS);
    }
    else if (!prefers_periphrastic_comparison(base_word)) {
        add_to_dictionary(build_regular_superlative(base_word), POS);
    }
}

void Lexicon::create_misc(const std::vector<std::string>& input) {
    for (std::size_t i = 1; i < input.size(); ++i) {
        add_to_dictionary(input[i], input[0]);
    }
}

std::string Lexicon::find_overload(const std::vector<std::string>& input, const std::string& key) const {
    for (const std::string& token : input) {
        const std::size_t equals_pos = token.find('=');
        if (equals_pos == std::string::npos) {
            continue;
        }

        const std::string candidate_key = token.substr(0, equals_pos);
        if (candidate_key == key) {
            return token.substr(equals_pos + 1);
        }
    }

    return "-";
}
