# Regular Expressions

This document gives a practical and theoretical overview of regular expressions as they are used in this project.

The lexer part of the project starts with regular expressions, converts them to NFA, then to DFA, and finally to lookup tables.

---

## Purpose

A regular expression describes a **regular language**, that is, a set of strings over a finite alphabet.

In this project, regular expressions are used to define token patterns for the lexer.

Examples of token patterns include:

- identifiers
- integers
- operators
- punctuation
- whitespace

A lexer uses these patterns to read a stream of characters and group them into tokens.

---

## Alphabet and Language

Let:

- `Σ` be a finite alphabet
- `Σ*` be the set of all finite strings over `Σ`
- `L` be a language, where `L ⊆ Σ*`

A regular expression defines such a language `L`.

Example:

- the regular expression `a`
  defines the language `{ "a" }`
- the regular expression `a|b`
  defines the language `{ "a", "b" }`
- the regular expression `ab`
  defines the language `{ "ab" }`

---

## Core Operators

The classical regular expression operators are:

### Literal

A symbol `a` in the alphabet matches itself.

Example:

```text
a
```

matches:

```text
"a"
```

---

### Concatenation

If `r` and `s` are regular expressions, then `rs` means:

- first match a string from `L(r)`
- then match a string from `L(s)`

Example:

```text
ab
```

matches:

```text
"ab"
```

---

### Alternation

If `r` and `s` are regular expressions, then:

```text
r | s
```

matches either `r` or `s`.

Example:

```text
a|b
```

matches:

```text
"a", "b"
```

---

### Kleene Star

If `r` is a regular expression, then:

```text
r*
```

matches zero or more repetitions of `r`.

Example:

```text
a*
```

matches:

```text
"", "a", "aa", "aaa", ...
```

This is called **Kleene star** or **Kleene closure**.

---

## Derived Operators

Some operators are often treated as shorthand.

### One or More

```text
r+
```

means one or more repetitions of `r`.

Equivalent form:

```text
rr*
```

Example:

```text
a+
```

matches:

```text
"a", "aa", "aaa", ...
```

---

### Optional

```text
r?
```

means zero or one occurrence of `r`.

Equivalent form:

```text
r | ε
```

Example:

```text
a?
```

matches:

```text
"", "a"
```

---

### Grouping

Parentheses are used to group subexpressions.

Example:

```text
(a|b)c
```

matches:

```text
"ac", "bc"
```

Without parentheses:

```text
a|bc
```

should mean either `a` or `bc` (this is not correct in the current code).

---

## Epsilon

The symbol `ε` denotes the **empty string**.

It is a string of length zero.

This is different from:

- an empty language
- a missing symbol
- whitespace

Example:

```text
ε
```

defines the language:

```text
{ "" }
```

Epsilon is important in automata construction, especially in NFA, where transitions may consume no input.

---

## Empty Language

The empty language is written as:

```text
∅
```

It contains no strings at all.

This is different from `ε`.

- `L(ε) = { "" }`
- `L(∅) = { }`

This distinction is important.

---

## Formal Definition

A regular expression over alphabet `Σ` can be defined recursively:

1. `∅` is a regular expression
2. `ε` is a regular expression
3. for every `a ∈ Σ`, `a` is a regular expression
4. if `r` and `s` are regular expressions, then:
   - `(r|s)` is a regular expression
   - `(rs)` is a regular expression
   - `(r*)` is a regular expression

Nothing else is a regular expression unless introduced as shorthand.

---

## Semantics

The meaning of a regular expression is the language it denotes.

Formally:

- `L(∅) = ∅`
- `L(ε) = { "" }`
- `L(a) = { "a" }` for `a ∈ Σ`
- `L(r|s) = L(r) ∪ L(s)`
- `L(rs) = { xy | x ∈ L(r), y ∈ L(s) }`
- `L(r*) = { x1x2...xn | n ≥ 0, each xi ∈ L(r) }`

The case `n = 0` means that `ε ∈ L(r*)`.

---

## Operator Precedence

Unless parentheses override the structure, regular expressions are usually parsed with this precedence:

1. `*`, `+`, `?`
2. concatenation
3. `|`

Examples:

```text
ab|c
```

means:

```text
(ab)|c
```

and not:

```text
a(b|c)
```

Example:

```text
a|bc*
```

means:

```text
a | (b(c*))
```

Correct operator precedence is essential for parsing regular expressions correctly.

---

## Examples

### Example 1

```text
a(b|c)*
```

This matches:

- `a`
- `ab`
- `ac`
- `abb`
- `acbc`
- `abcbcc`
- and so on

---

### Example 2

```text
digit digit*
```

where `digit` means one of `0` through `9`.

This describes a non-empty sequence of digits.

Equivalent shorthand:

```text
digit+
```

---

### Example 3

```text
letter (letter|digit)*
```

This is a typical pattern for an identifier:

- first character must be a letter
- following characters may be letters or digits

---

## Regular Expressions and Lexing

In this project, regular expressions are not used as a final runtime notation. They are an intermediate formal description.

The pipeline is:

```text
Regular Expression
    → NFA
    → DFA
    → table-driven lexer
```

So the regular expression is the starting point for code generation.

---

## Relation to Finite Automata

Regular expressions, NFA, and DFA are equivalent in expressive power.

They define exactly the same class of languages: the regular languages.

This means:

- every regular expression can be converted to an NFA
- every NFA can be converted to a DFA
- every DFA can be represented by some regular expression

In this project, the important direction is:

```text
Regular Expression → NFA → DFA
```

---

## Limits of Regular Expressions

Regular expressions can only describe regular languages.

They are suitable for lexical structure, but not for arbitrary syntax.

They cannot in general express:

- nested parentheses of unbounded depth
- recursive grammatical structure

That is why parsing requires context-free grammar and a parser.

In other words:

- lexer: regular languages
- parser: context-free languages

---

## Practical Notes for This Project

When implementing regular expressions for lexer generation, the following issues are especially important:

### 1. Distinguish `ε` from `∅`

They are not the same and must not be handled the same way.

### 2. Get operator precedence right

Many bugs come from incorrect parsing of:

- concatenation
- alternation
- star / plus / optional

### 3. Make concatenation explicit internally

Even if concatenation is implicit in the source syntax, it is often useful to represent it explicitly in the syntax tree.

### 4. Define the supported syntax clearly

For example:

- Is `+` supported directly?
- Is `?` supported directly?
- Are character classes supported?
- Are escape sequences supported?
- Is `.` supported as “any character”?

### 5. Keep theory and implementation aligned

The implementation should reflect the formal structure of regular expressions, since this makes NFA construction easier to verify.

---

## Minimal Internal Representation

A regular expression is often easiest to process if first converted into a syntax tree.

Typical node kinds are:

- empty language
- epsilon
- literal
- concatenation
- alternation
- star

Optional derived nodes:

- plus
- optional

---

## Summary

Regular expressions are the formal starting point of the lexer.

They:

- describe regular languages
- define token patterns
- are built from a small set of operators
- can be translated into NFA and DFA
- are powerful enough for lexing, but not for full parsing

In this project, regular expressions belong to the lexer stage and should be kept conceptually separate from context-free grammar and parser generation.