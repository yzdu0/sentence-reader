#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include "sentence-reader/Lexicon.h"

Word::Word() {
	word = "";
	POS = "";
}

Word::Word(std::string word, std::string POS) :
	word(word), POS(POS) {

	if (this->POS.back() == ':') {
		this->POS.pop_back();
	}
}

Lexicon::Lexicon() {
	std::ifstream file("data/vocab.txt");
	std::string str;

	while (std::getline(file, str)) {
		if (str.size() == 0) continue;

		if (str[0] == '#') continue;

		std::stringstream ss(str);
		std::string cur;
		std::vector<std::string> line;
		while (getline(ss, cur, ' ')) {
			if (cur != "|") {
				line.push_back(cur);
			}
		}//

		if (line[0] == "N:") {
			create_nouns(line);//
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

void Lexicon::add_to_dictionary(std::string word, std::string POS) {
	// Word doesn't exist
	if (dictionary.find(word) == dictionary.end()) {
		dictionary[word] = { Word(word, POS) };
	}
	else {
		dictionary[word].push_back(Word(word, POS));
	}
}

bool Lexicon::search_word(std::string word, std::string POS) {
	for (const Word& it : dictionary[word]) {
		if (it.word == word && it.POS == POS) {
			return true;
		}
	}
	return false;
}

void Lexicon::create_nouns(std::vector<std::string> input) {
	std::string base_word = input[1];
	std::string POS = input[0];
	add_to_dictionary(base_word, POS);

	std::string plural_version = find_overload(input, "plural");

	// Plural - just tack an S on the end if no overload exists
	if (plural_version == "-") {
		plural_version = base_word;
		plural_version.push_back('s');
	}
	add_to_dictionary(plural_version, POS);
}

void Lexicon::create_verbs(std::vector<std::string> input) {
	std::string POS = input[0];
	std::string base_word = input[1];
	add_to_dictionary(base_word, POS);

	// Past tense - add an -ed on the end. (Or just -d if it already ends with e).
	std::string past_tense = find_overload(input, "past");
	if (past_tense == "-") {
		past_tense = base_word;
		if (past_tense.back() != 'e') past_tense.push_back('e');
		past_tense.push_back('d');
	}
	add_to_dictionary(past_tense, POS);

	// Present tense - tack a -ing on the end. Absorb e if it's the last letter.
	std::string present_tense = find_overload(input, "pres3");
	if (present_tense == "-") {
		present_tense = base_word;
		if (present_tense.back() == 'e') present_tense.pop_back();
		present_tense.push_back('i');
		present_tense.push_back('n');
		present_tense.push_back('g');
	}
	add_to_dictionary(present_tense, POS);
}

void Lexicon::create_adjectives(std::vector<std::string> input) {
	std::string base_word = input[1];
	add_to_dictionary(base_word, input[0]);
}

void Lexicon::create_misc(std::vector<std::string> input) {
	for (std::size_t i = 1; i < input.size(); i++) {
		add_to_dictionary(input[i], input[0]);
	}
}

std::string Lexicon::find_overload(const std::vector<std::string>& input, const std::string& key) {
	for (const std::string& token : input) {
		std::string candidate_key = "";
		std::size_t pos = 0;
		for (std::size_t i = 0; i < token.size(); i++) {
			if (token[i] == '=') {
				pos = i + 1;
				break;
			}
			else {
				candidate_key.push_back(token[i]);
			}
		}
		std::string res = "";
		if (candidate_key == key) {
			for (std::size_t j = pos; j < token.size(); j++) {
				res.push_back(token[j]);
			}
			return res;
		}
	}
	return "-";
}//
/*
TODO:
Verb past tense, present tense, gerund, past participle

Noun plural

Adjective comparative, superlative

Given a string, we should be able to look up the word, and determine its information. 
(At the very least, its part of speech, which we already have.)

When generating words, we should also create all of the related words (e.g. the noun "bird" also 
creates the noun "birds".  ). 
*/