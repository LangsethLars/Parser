#pragma once

#include "LexerTypes.h"



class LexerBase {

public:

	LexerBase(const TokenClasses &tc, const LexerTable &lt);

	bool lexFile(const char* filename);
	void debug();

protected:

	bool readFile(const char* filename);
	int getTokenClassId(const char *name);
	void displayErrorMessage(const uint8_t* pTokenStart, const uint8_t* pTokenErr, int tokenLineNumber);

	// Fixed table driven lexer
	const LexerTable &m_LexerTable;

public:

	// Fixed token classes
	const TokenClasses& m_TokenClasses;

	// Result from lexFile()
	RawText m_RawText;
	TokenSequence m_TokenSequence;

}; // End of class LexerBase
