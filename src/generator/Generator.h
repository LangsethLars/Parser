#pragma once

#include "../parser/Bootstrap_Parser.h"
#include "../parser/ParseTreeIterator.h"
#include "BuildNFA.h"
#include "BuildCFG.h"



class Generator {

public:

	Generator() :
		m_StatusOk(true),
		m_MainNFA(),
		m_MainCFG(m_MainNFA.m_TokenClasses)
	{
	}

	bool makeCodeFromScript(const char *rootName, bool bOnlyLexer = false);

private:

	void clear() {
		m_StatusOk = true;
		m_TempNFA.clear();
		m_MainNFA.clear();
		m_MainCFG.clear();
	}

	bool buildLexer(const char* rootName);
	void getTokenExpr(ParseTreeIterator it, ParseTreeIterator itLast, BuildNFA &nfa);
	unsigned char getChar(int &i);

	bool buildParser(const char* rootName);

private:

	bool m_StatusOk;

	// Will be cleared in makeCodeFromScript() when parsing a new input file.
	Bootstrap_Parser m_Parser;

	// Used when building tables from an input file
	std::map<std::string, BuildNFA> m_TempNFA;
	BuildNFA m_MainNFA;
	BuildCFG m_MainCFG;
};
