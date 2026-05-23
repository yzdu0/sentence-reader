#include <sentence-reader/Grammar.h>
#include <vector>

Grammar::Grammar() {
    std::vector<std::string> rules_string = {
        "S0 -> S",
        "S -> NP VP", // Basic declarative sentence: subject + predicate
        "S -> S Conj S",
        "S -> AdvP S", // Sentence-initial adverbs: "suddenly the dog barked"
        "S -> PP S", // Fronted prepositional phrase: "in the morning the teacher smiled"
        "S -> VP", // Imperatives and verbless commands: "run"

        "Conj -> CoordConj",
        "Conj -> SubordConj",
        "Conj -> ConjAdv",

        "CoordConj -> AND",
        "CoordConj -> NOR",
        "CoordConj -> BUT",
        "CoordConj -> OR",
        "CoordConj -> YET",
        "CoordConj -> SO",

        "ConjAdv -> THEREFORE",

        /*"Conj -> AND",
        "Conj -> OR",
        "Conj -> BUT",
        "Conj -> NOR",
        "Conj -> YET",
        "Conj -> AND",
        "Conj -> SO",
        "Conj -> THEREFORE",*/

        "VP -> V", // Intransitive verb
        "VP -> V NP", // Transitive verb
        "VP -> VP PP", // VP post-modification by a prepositional phrase: "eat pizza in the park"
        "VP -> VP AdvP", // VP post-modification by an adverb: "walk quickly"
        "VP -> AdvP VP", // VP pre-modification by an adverb: "quietly walked home"
        "VP -> VP CoordConj VP", // Coordinated verb phrases: "ran and jumped"

        "VP -> V Inf",
        "VP -> AuxP VP", // Auxiliary + verbal predicate: "can see the dog"
        "VP -> AuxP AdjP", // Auxillary phrase, aux + optional negation
        "VP -> AuxP NP",
        "VP -> CopP AdjP", // Copular phrase, copular + optional negation
        "VP -> CopP NP",
        "VP -> CopP PP", // Copular + PP: "is in the garden"
        "VP -> CopP AdvP", // Copular + adverb: "is here"

        "NP -> Pron",
        "NP -> Name",
        "NP -> Det N",
        "NP -> Det AdjP N",
        "NP -> Det Num N",
        "NP -> Det Num AdjP N",
        "NP -> AdjP N",
        "NP -> Num N",
        "NP -> Num AdjP N",

        "NP -> NP PP",
        "NP -> NP RelClause",
        "NP -> N",
        "NP -> NP CoordConj NP", // coordinated noun phrases: "cats and dogs"


        "AdjP -> Adj",
        "AdjP -> Adj AdjP", // Multiple adjectives behave the same as one
        "AdjP -> AdvP AdjP", // Adverb-modified adjectives: "very calm"

        "AdvP -> Adv",
        "AdvP -> Adv AdvP",

        "AuxP -> Aux",
        "AuxP -> Aux Neg", // Auxiliary with negation

        "CopP -> Cop",
        "CopP -> Cop Neg", // Copular verb with negation

        "PP -> P NP",
        "PP -> FOR NP Inf",

        "RelClause -> RelPron VP",

        "Inf -> TO VP",

        "TO -> to",

        "FOR -> for",
    };

    std::vector<std::string> negation_rules_string = {
        "NP VP <-> NP Neg(VP)",
        "S and S <-> Neg(S) or Neg(S)", 
        "S or S <-> Neg(S) and Neg(S)",
        //"S therefore S <-> "
    };
    // I saw the man and I drank a coffee :: I never saw the man or I never drank a coffee
    for (std::string rule : rules_string) {
        rules.push_back(Rule(rule));
    }

    std::vector<std::string> rules_string2 = {
        "S -> NP VP",

        "VP -> V",
        "VP -> V NP",
        "VP -> VP PP",
        "VP -> V NP PP",

        "NP -> Pron",
        "NP -> Det N",
        "NP -> NP PP",
        "NP -> N",

    };

    for (std::string rule : rules_string2) {
        basic_rules.push_back(Rule(rule));
    }
}

Grammar Grammar::ChomskyNormalForm(Grammar input) {
    Grammar g;
    return g;
}

void Grammar::enumerate() {

}
