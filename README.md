# Parser
Simple project to generate lexer and LL(1) parser tables from a text file.
No fancy error messages and no fancy binary format for output. See **The Dragon Book** <https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools>
and **Introduction to Automata Theory, Languages, and Computation** <http://infolab.stanford.edu/~ullman/ialc.html> for details on the theory.
**Coursera** <https://www.coursera.org/> had two courses covering this theory: **Automata** by Jeffrey Ullman, and **Compilers** by Alex Aiken.

This project is meant as part of a bigger project **MetaMathVerifier** that will read mathematical proofs from a text file in a simple mathematical language and verify the proof.
Here are some useful links about this subject:
* <https://github.com/david-a-wheeler/metamath-test>
* <http://us.metamath.org/>
* <http://www.cs.miami.edu/~tptp/>

LexGenerator will read a text file that uses regular expressions (RE) to define tokens.
The RE will be converted into a Nondeterministic Finite Automaton (NFA).
The NFA will be converted into a Deterministic Finite Automaton (DFA).
This DFA can be stored as a binary lookup table.
This binary file can be used by a stand alone Lexer to convert a text file to a list of tokens.

RE :arrow_right: NFA :arrow_right: DFA :arrow_right: Binary lookup table stored in a file :arrow_right: Lexer

Text file :arrow_right: Lexer with table that specifies how to scan the file :arrow_right: List of tokens

So far the following are finished:
* Main.cpp - Just to start the code.
* NFA.h and NFA.cpp - Nondeterministic Finite Automaton.
* ChSet.h - Just a helper class for NFA.
* DFA.h and DFA.cpp - Deterministic Finite Automaton.
* BitSet.h and BitSet.cpp - Helper class to convert NFA to DFA (remember multiple states in NFA).
* LexGenerator.h and LexGenerator.cpp - Bootstrap to create the first .dfa file to drive a Lexer.
* Lexer.h and Lexer.cpp - A stand alone class for transforming a file into a list of tokens (to be used by a parser).

Next step will be to implement a hand written parser to make it possible to make other lex specifications than LexBootstrap.dfa.

The project is created in Visual Studio 2017. Most windows specific things has been removed just in case somebody need to run it on another platform. The remaining VS2017 spesific files are:
* Parser.sln - 
  Solution file for the project. The main entry point for Visual Studio 2017.
* Parser.vcxproj - 
  Contains information about the version of Visual C++ that generated the file, and information about the platforms, configurations, and project features selected with the Application Wizard.
* Parser.vcxproj.filters - 
  Contains information about the association between the files in the project and the filters. This association is used in the IDE to show grouping of files with similar extensions under a specific node (for e.g. ".cpp" files are associated with the "Source Files" filter).
