#pragma once

#include "LexerTypes.h"

#include <string>
#include <vector>



struct VariableClass {
	VariableClass(const char *variableName, bool skipToken = false)
		: name(variableName), skip(skipToken) { }

	std::string name;
	bool skip;
};

using VariableClasses = std::vector<VariableClass>;



struct Production {
	int variableClassId;
	std::vector<int> symbols; // -(n+1) means variableClassId #n, else the entry is tokenClassId
};

using Productions = std::vector<Production>;

// Example with two productions:
// -----------------------------
// <>      = <Start> END_OF_FILE
// <Start> = SEMICOLON
//
// {
//    { 0, { -2, 0 } },  // <>      has id = 0 , { -2 = -(1+1) variableClassId 1 is <Start>, 0 = tokenClassId for END_OF_FILE }
//    { 1, { 1 } }       // <Start> has id = 1 , { 1 = tokenClassId for COMMENT }
// }

using ParsingTable = std::vector<int>;   // ParsingTable[ #VariableClasses , #TokenClasses ] 2D table pointing to production #



struct ParseNode {
	int symbolIdOrTokenSequenceNo; // -(n+1) means variableClassId #n, else token #n in sequence of tokens

	int parent;
	int firstChild;
	int lastChild;

	int prev; // -1 ==> First symbol of this local level
	int next; // -1 ==> Last symbol of this local level
};

using ParseTree = std::vector<ParseNode>;
