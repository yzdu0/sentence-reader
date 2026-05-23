# sentence-reader

A C++ project that parses English sentences and reports one or more syntactic interpretations.
No machine learning involved.

## Improvements in this version

- The parser now builds cleanly with Clang on macOS.
- Input is normalized before parsing, so punctuation like commas and periods no longer break valid sentences.
- One-shot CLI parsing is supported with `--sentence`, which makes the project easier to test and script.
- Parse output now reports how many interpretations were found and numbers each one.
- Lexicon inflection generation now creates real third-person singular verb forms such as `walks`.
- Basic CTest coverage checks ambiguity, punctuation handling, and inflected verbs.

## Build

```bash
cmake -S main -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Examples

```bash
./build/sentence-reader --sentence "I saw the man with the telescope"
```

Example output:

```text
Input: i saw the man with the telescope
Found 2 interpretation(s).
[1] (S0 ...)
[2] (S0 ...)
```

```bash
./build/sentence-reader --sentence "All humans are mortal, therefore I am not human."
```

The parser also still supports interactive mode:

```bash
./build/sentence-reader
```
