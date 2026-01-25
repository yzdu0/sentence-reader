#pragma once
#include <string>
#include <vector>
#include <unordered_map>
class Word {
public:
	std::string word;
	std::string POS;
	/*enum PartOfSpeech {
		Verb,
		Noun,
		Adjective,
		Pronoun,
		Determiner,
		Preposition,
		Conjunction,
		Negation,
		AuxillaryVerb,
		Adverb,
		CopulaVerb,

		TO,
		FOR,
	};*/

	/*enum Plurality {
		NA,
		Singular,
		Plural,
	
	};

	enum Tense {
		NA,
		Past,
		Present,
		Future,
	};*/

	//PartOfSpeech pos;
	//Plurality plurality = Plurality::NA;
	//Tense tense = Tense::NA;

	Word();

	Word(std::string word, std::string POS);

	//std::string get_POS();
};

class Lexicon {
public:
	Lexicon();

	std::unordered_map<std::string, std::vector<Word>> dictionary;

	bool search_word(std::string word, std::string POS);

private:

	void add_to_dictionary(std::string word, std::string POS);

	void create_nouns(std::vector<std::string> input);

	void create_verbs(std::vector<std::string> input);

	void create_adjectives(std::vector<std::string> input);

	void create_misc(std::vector<std::string> input);

	std::string find_overload(
		const std::vector<std::string>& input, 
		const std::string& key
	);
};