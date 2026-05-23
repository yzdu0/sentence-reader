#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class Word {
public:
    std::string word;
    std::string POS;

    Word();

    Word(std::string word, std::string POS);
};

class Lexicon {
public:
    Lexicon();

    std::unordered_map<std::string, std::vector<Word>> dictionary;

    bool search_word(const std::string& word, const std::string& POS) const;

    bool contains_word(const std::string& word) const;

private:
    void add_to_dictionary(const std::string& word, const std::string& POS);

    void create_nouns(const std::vector<std::string>& input);

    void create_verbs(const std::vector<std::string>& input);

    void create_adjectives(const std::vector<std::string>& input);

    void create_misc(const std::vector<std::string>& input);

    std::string find_overload(const std::vector<std::string>& input, const std::string& key) const;
};
