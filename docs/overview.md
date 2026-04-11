# Overview

This project implements a generator for lexer and LL(1) parser tables based on a textual grammar specification.

The overall goal is to build a foundation for a larger project (**Proof**) that will parse and verify mathematical proofs written in a formal language.

---

## Architecture

The system follows the classical compiler design pipeline:

```
Regular Expressions
    ↓
NFA (Non-deterministic Finite Automaton)
    ↓
DFA (Deterministic Finite Automaton)
    ↓
Lookup Tables (Lexer)

Context-Free Grammar (BNF/EBNF)
    ↓
FIRST / FOLLOW sets
    ↓
LL(1) Parsing Table
    ↓
Parser
```

---

## Scope

The project consists of two main parts:

### 1. Lexer

* Based on **regular expressions**
* Transformed into **NFA → DFA**
* Compiled into **table-driven state machine**

### 2. Parser

* Based on **context-free grammar**
* Uses **LL(1) parsing**
* Generates parsing tables

---

## Bootstrapping

The project includes a bootstrap process:

1. A manually generated parser (**Bootstrap_Lexer**) and (**Bootstrap_Parser**) is created, moved into src/ and activated
2. This parser reads a grammar file (`Bootstrap.txt`)
3. It generates a new parser (**Bootstrap_Lexer**) and (**Bootstrap_Parser**)
4. The generated parser should be equivalent to the bootstrap version

This helps to verify correctness of the generator and the file (`Bootstrap.txt`).

---

## Theoretical Background

The project is based on standard compiler theory:

* Regular languages and finite automata
* Context-free grammars
* LL(1) parsing

Recommended references:

* *Compilers: Principles, Techniques, and Tools* (Dragon Book)
* *Introduction to Automata Theory* (Ullman)

---

## Motivation

The goal is not just to build a working parser, but to fully understand:

* The relationship between formal language theory and implementation
* How compilers and interpreters are constructed
* How parsing can be applied to formal mathematical languages

This serves as preparation for more advanced topics such as:

* Automated theorem proving
* Formal verification
* AI-assisted reasoning systems
