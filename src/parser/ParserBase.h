#pragma once

#include "ParserTypes.h"
#include "LexerBase.h"

#include <string>
#include <vector>



class ParserBase {

public:

	ParserBase(
		const VariableClasses &vc,
		const Productions &prod,
		const ParsingTable &pt,
		const TokenClasses &tc,
		const TokenSequence &ts,
		const RawText &rt
	);

	bool parseTokenSequence();

	void displayErrorMessage(const unsigned char *pTokenStart, const unsigned char *pTokenErr, int tokenLineNumber);
	void debug();

private:

	int getTokenClassId(const char *name);
	int getVariableClassId(const char *name);

public:

	const VariableClasses &m_VariableClasses;
	const TokenClasses& m_TokenClasses;
	const TokenSequence& m_TokenSequence;
	const RawText& m_RawText;

private:

	// Fixed table driven parser
	const Productions &m_Productions;
	const ParsingTable &m_ParsingTable;

public:

	ParseTree m_ParseTree;

}; // End of class ParserBase
