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

Parser is not finished but it works. Better error handling should be implemented. The bootstrapper was implemented first. This was used to generate two files **Parser_Bootstrap.cpp** and **Parser_Bootstrap.h**.
These files was then used for lexing and parsing the text file **DefaultCFG.txt** to generate **Parser_DefaultCFG.cpp** and **Parser_DefaultCFG.h**. They are equal to the other two files as they should be.
The text file is meant to represent what was done during the bootstrap. The files **Parser.cpp** and **Parser.h** is meant to be used stand alone together with the generated files with lookup tables.

The project is created in Visual Studio 2017. Most windows specific things has been removed just in case somebody need to run it on another platform. The remaining VS2017 spesific files are:
* Parser.sln - 
  Solution file for the project. The main entry point for Visual Studio 2017.
* Parser.vcxproj - 
  Contains information about the version of Visual C++ that generated the file, and information about the platforms, configurations, and project features selected with the Application Wizard.
* Parser.vcxproj.filters - 
  Contains information about the association between the files in the project and the filters. This association is used in the IDE to show grouping of files with similar extensions under a specific node (for e.g. ".cpp" files are associated with the "Source Files" filter).

PS: Remember to change Project-->Properties so the program runs in the **Test** folder.