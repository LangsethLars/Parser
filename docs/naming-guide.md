# Naming Guide

This document defines the naming conventions used in this project.

The goal is consistency, readability, and a clear distinction between handwritten code, generated code, and documentation.

---

## General Principles

Naming should make it easy to see:

- what is handwritten
- what is generated
- what belongs to documentation
- what belongs to source code

Consistency is more important than choosing a fashionable style.

---

## Documentation Files

Documentation files in `docs/` use:

- lowercase
- kebab-case
- English names

Examples:

- `overview.md`
- `naming-guide.md`
- `regular-expressions.md`
- `subset-construction.md`

This style is well suited for Markdown, GitHub, and URLs.

---

## Handwritten Source Files

Handwritten C++ source and header files use:

- `PascalCase` filenames
- `.cpp` for implementation files
- `.h` for header files

Examples:

- `BuildNFA.cpp`
- `BuildDFA.cpp`
- `BitSet.h`
- `Parser.cpp`

This matches the style already used in the project and fits well with traditional C++ and Visual Studio conventions.

---

## Generated Source Files

Generated parser files use the form:

- `Parser_XXX.cpp`
- `Parser_XXX.h`

where `XXX` is derived from the input text file used to generate them.

- `Parser_XXX.txt`

Examples:

- `Parser_Bootstrap.txt`
- `Parser_Bootstrap.cpp`
- `Parser_Bootstrap.h`

- `Parser_Proof.txt`
- `Parser_Proof.cpp`
- `Parser_Proof.h`

The underscore is intentional in this case.

It is used to separate:

- the fixed generated file prefix: `Parser`
- the specific grammar or source name: `XXX`

This is not considered an inconsistency. It is a meaningful naming pattern for generated artifacts.

---

## Examples

### Good

- `BuildNFA.cpp`
- `BuildParseTable.cpp`
- `BitSet.h`
- `Parser.cpp`
- `Parser_Bootstrap.cpp`
- `Parser_Proof.h`
- `overview.md`
- `nfa-to-dfa.md`

### Avoid

- `buildnfa.cpp`
- `build_nfa.cpp`
- `Build_NFA.cpp`
- `Overview.md`
- `RegularExpressions.md`

---

## Future Extensions

If additional generated file families are introduced, they should follow the same pattern:

- fixed prefix
- underscore
- source-specific suffix

Example:

- `Lexer_DefaultRE.cpp`
- `Grammar_Arithmetic.cpp`

Only use this pattern when the file is generated and the suffix identifies its source.

---

## Summary

This project uses three naming styles:

- `PascalCase` for handwritten C++ code
- `Parser_XXX` for generated parser files
- `kebab-case` for Markdown documentation

This keeps the project readable while preserving the meaning of generated artifacts.