# Parser

A simple project for generating lexer and LL(1) parser tables from a textual grammar specification.

The project is part of a larger goal: building a system for parsing and verifying mathematical proofs written in a formal language.

---

## Features

* Regular expression → NFA → DFA pipeline (lexer)
* Table-driven lexer generation
* LL(1) parser table generation
* Bootstrap process for self-generation and verification

---

## Documentation

Detailed documentation is available in the `docs/` folder:

* [Overview](docs/overview.md)
* [Regular Expressions](docs/regular-expressions.md)
* [Naming Guide](docs/naming-guide.md)

More documentation will be added over time.

---

## Getting Started

The project is developed using Visual Studio.

1. Open `Parser.sln`
2. Build the project
3. Set the working directory to $(ProjectDir) (Project → Parser Properties)

---

## Status

The project is functional but not complete.

* Core functionality works
* Error handling is minimal
* Documentation is being improved

---

## References

* *Compilers: Principles, Techniques, and Tools* (Dragon Book)
* *Introduction to Automata Theory, Languages, and Computation* (Ullman)
