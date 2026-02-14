A C++ project that parses English sentences. 
No machine learning involved.

TODO:
- Add more words
- Better formatting, especially for ambiguous sentences
- Generating true statements deduced from the sentence
  - Generate negation of a sentence

Example output:

```
Enter a sentence:
 >I saw the man in the park with my telescope
5 interpretations discovered:

S
|   NP
|   |   Pron -> i
|   VP
|   |   VP
|   |   |   VP
|   |   |   |   V -> saw
|   |   |   |   NP
|   |   |   |   |   Det -> the
|   |   |   |   |   N -> man
|   |   |   PP
|   |   |   |   P -> in
|   |   |   |   NP
|   |   |   |   |   Det -> the
|   |   |   |   |   N -> park
|   |   PP
|   |   |   P -> with
|   |   |   NP
|   |   |   |   Det -> my
|   |   |   |   N -> telescope
----
S
|   NP
|   |   Pron -> i
|   VP
|   |   VP
|   |   |   V -> saw
|   |   |   NP
|   |   |   |   NP
|   |   |   |   |   Det -> the
|   |   |   |   |   N -> man
|   |   |   |   PP
|   |   |   |   |   P -> in
|   |   |   |   |   NP
|   |   |   |   |   |   Det -> the
|   |   |   |   |   |   N -> park
|   |   PP
|   |   |   P -> with
|   |   |   NP
|   |   |   |   Det -> my
|   |   |   |   N -> telescope

...
```
