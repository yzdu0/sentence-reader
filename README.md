A C++ project that parses English sentences. 
No machine learning involved.

TODO:
- Add more words
- Better formatting, especially for ambiguous sentences

Example output:

```
Enter a sentence:
> All humans are mortal, therefore I am not human
Syntax Tree:
[all humans are mortal therefore i am not human] [S -> S Conj S]
|   [all humans are mortal] [S -> NP VP]
|   |   [all humans] [NP -> Det N]
|   |   |   [all] [Det -> all]
|   |   |   [humans] [N -> humans]
|   |   [are mortal] [VP -> Cop AdjP]
|   |   |   [are] [Cop -> are]
|   |   |   [mortal] [AdjP -> Adj]
|   |   |   |   [mortal] [Adj -> mortal]
|   [therefore] [Conj -> therefore]
|   [i am not human] [S -> NP VP]
|   |   [i] [NP -> Pron]
|   |   |   [i] [Pron -> i]
|   |   [am not human] [VP -> Aux Neg NP]
|   |   |   [am] [Aux -> am]
|   |   |   [not] [Neg -> not]
|   |   |   [human] [NP -> N]
|   |   |   |   [human] [N -> human]
(S0 (S (S (NP (Det "all") (N "humans")) (VP (Cop "are") (AdjP (Adj "mortal")))) (Conj "therefore") (S (NP (Pron "i")) (VP (Aux "am") (Neg "not") (NP (N "human"))))))
```
