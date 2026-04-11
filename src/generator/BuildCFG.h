#pragma once

#include "BitSet.h"
#include "../parser/LexerTypes.h"
#include "../parser/ParserTypes.h"

#include <string>
#include <vector>



class BuildCFG {

public:

	BuildCFG(TokenClasses &tokenClasses) :
		m_TokenClasses(tokenClasses),
		m_StatusOk(true) {
		//clear();
	}

	void clear() {
		m_VariableClasses.clear();
		m_ExtendVariables.clear();
		m_Productions.clear();
		m_ExtendProductions.clear();
		m_ParsingTable.clear();

		m_StatusOk = true;
	}

	BuildCFG& addProduction(const char *variableName);
	BuildCFG& addSymbol(int symbolId) { m_Productions.back().symbols.emplace_back(symbolId); return *this; }
	BuildCFG& addTerminal(const char *tokenName);
	BuildCFG& addVariable(const char *variableName);

	bool setVariableClassToSkip(const char* variableName);

	bool buildParseTable();
	bool saveCFG(const char* rootName);	// Used to save the parse tables created by buildParseTable() to files that can be used by the parser.

	bool isOk() const { return m_StatusOk; }

private:

	void initBuildParameters();
	bool findEmpty();
	bool findFirst();
	bool findFollow();

	// Fixed token/variable classes

	TokenClasses &m_TokenClasses;

	// Local definition of parser

	VariableClasses m_VariableClasses;

	struct ExtendVariable {
		bool m_CanBeEmpty;
		BitSet m_FirstTerminals;
		BitSet m_FollowTerminals;

	};
	std::vector<ExtendVariable> m_ExtendVariables;

	Productions m_Productions;

	struct ExtendProduction {
		bool m_CanBeEmpty;
		BitSet m_FirstTerminals;
	};
	std::vector<ExtendProduction> m_ExtendProductions;

	ParsingTable m_ParsingTable;

	// Debugging and error handling
	bool m_StatusOk;

};
