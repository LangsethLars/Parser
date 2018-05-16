#pragma once

#include "Parser.h"
#include "BuildNFA.h"
#include "BuildParser.h"



class Generator {

public:

	Generator() :
		m_StatusOk(true),
		m_ParserDef(m_TokenClasses, m_VariableClasses, m_Productions) {
	}

	bool bootstrap();
	bool buildTablesFrom(char *rootName);

private:

	void clear() {
		m_StatusOk = true;
		m_TempNFA.clear();
		m_MainNFA.clear();
		m_ParserDef.clear();
		m_TokenClasses.clear();
		m_VariableClasses.clear();
		m_LexerTable.clear();
		m_Productions.clear();
		m_ParsingTable.clear();
	}

	bool bootstrapLexer();
	bool addNFA(BuildNFA &nfa, const char *tokenName, bool ignoreToken = false);

	bool bootstrapParser();
	bool setVariableClassToSkip(const char *variableName);

	bool buildLexer();
	bool getTokenExpr(NodeIter it, NodeIter itEnd, BuildNFA &nfa);
	unsigned char getChar(int &i);

	bool buildParser();

	bool saveTables(char *rootName);
	bool readCfgFile(char *rootName, RawText &rawText);

	bool m_StatusOk;

	// Used when building tables from an input file
	Parser m_Parser;
	std::map<std::string, BuildNFA> m_TempNFA;

	BuildNFA m_MainNFA;
	BuildParser m_ParserDef;

	TokenClasses m_TokenClasses;
	VariableClasses m_VariableClasses;
	LexerTable m_LexerTable;
	Productions m_Productions;
	ParsingTable m_ParsingTable;
};

