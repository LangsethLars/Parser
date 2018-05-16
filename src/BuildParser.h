#pragma once

#include "BitSet.h"
#include "Parser.h"

#include <string>
#include <vector>



class BuildParser {

public:

	BuildParser(TokenClasses &tokenClasses, VariableClasses &variableClasses, Productions &productions) :
		m_TokenClasses(tokenClasses),
		m_VariableClasses(variableClasses),
		m_Productions(productions),
		m_StatusOk(true) {
		//clear();
	}

	void clear() {
		m_VariableClasses.clear();
		m_ExtendVariables.clear();
		m_Productions.clear();
		m_ExtendProductions.clear();
		m_StatusOk = true;
	}

	BuildParser &addProduction(const char *variableName);
	BuildParser &addSymbol(int symbolId) { m_Productions.back().symbols.emplace_back(symbolId); return *this; }
	BuildParser &addTerminal(const char *tokenName);
	BuildParser &addVariable(const char *variableName);

	bool buildParceTable(ParsingTable &parsingTable);

	bool isOk() const { return m_StatusOk; }

private:

	void initBuildParameters();
	bool findEmpty();
	bool findFirst();
	bool findFollow();

	// Fixed token/variable classes

	TokenClasses &m_TokenClasses;
	VariableClasses &m_VariableClasses;
	Productions &m_Productions;

	// Local definition of parser

	struct ExtendVariable {
		bool m_CanBeEmpty;
		BitSet m_FirstTerminals;
		BitSet m_FollowTerminals;

	};
	std::vector<ExtendVariable> m_ExtendVariables;

	struct ExtendProduction {
		bool m_CanBeEmpty;
		BitSet m_FirstTerminals;
	};
	std::vector<ExtendProduction> m_ExtendProductions;

	// Debugging and error handling
	bool m_StatusOk;

};
