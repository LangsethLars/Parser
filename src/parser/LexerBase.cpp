#include "LexerBase.h"

#include <fstream>
#include <iterator>
#include <cstdio>


LexerBase::LexerBase(const TokenClasses &tc, const LexerTable &lt) :
	m_TokenClasses(tc),
	m_LexerTable(lt)
{
}



bool LexerBase::lexFile(const char* filename)
{
	Token token;
	m_TokenSequence.clear();

	m_RawText.clear();
	if (!readFile(filename)) {
		printf("LexerBase::lexFile failed to readFile(\"%s\")\n", filename);
		return false;
	}

	const uint8_t* pRawText = m_RawText.data();
	const uint8_t* p = pRawText;
	const uint8_t* pEnd = pRawText + m_RawText.size();

	int lineNumber = 1;

	while (p < pEnd) {

		int stateNo = 0;
		int tokenId = -1;
		int tokenLineNumber = lineNumber;

		const uint8_t *pTokenStart = p;
		const uint8_t *pTokenEnd = p;
		const uint8_t *pTokenErr = p;

		while (p < pEnd) {

			uint8_t ch = *p++;

			if ((stateNo = m_LexerTable[stateNo].onChMove[ch]) < 0) {
				pTokenErr = p;
				p = pTokenEnd;
				break;
			}

			if (m_LexerTable[stateNo].tokenClassId > 0) {
				pTokenEnd = p;
				tokenId = m_LexerTable[stateNo].tokenClassId;
			}

		}

		// Count line numbers after token is extracted
		for (const uint8_t* pLine = pTokenStart; pLine < pTokenEnd; ++pLine) {
			if (*pLine == '\n') {
				++lineNumber;
			} else if (*pLine == '\r') {
				// Handle CR or CRLF
				if (pLine + 1 < pTokenEnd && *(pLine + 1) == '\n') {
					++pLine; // Skip the LF in CRLF
				}
				++lineNumber;
			}
		}

		// Simple error checking
		if (tokenId < 0 || tokenId >= int(m_TokenClasses.size())) {
			displayErrorMessage(pTokenStart, pTokenErr, tokenLineNumber);
			return false;
		}

		if (!m_TokenClasses[tokenId].ignore) {
			token.tokenClassId = tokenId;
			token.lineNumber = tokenLineNumber;
			token.lexemeStart = pTokenStart - pRawText;
			token.lexemeLength = pTokenEnd - pTokenStart;
			m_TokenSequence.push_back(token);
		}

	}

	// End with an END_OF_FILE token (id = 0)
	token.tokenClassId = 0;
	token.lineNumber = lineNumber;
	token.lexemeStart = 0;
	token.lexemeLength = 0;
	m_TokenSequence.push_back(token);

	return true;
}



int LexerBase::getTokenClassId(const char *name)
{
	for (size_t i = 0; i < m_TokenClasses.size(); ++i)
		if (m_TokenClasses[i].name == name)
			return int(i);

	return -1;
}



bool LexerBase::readFile(const char* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	
	if (!file.is_open()) {
		printf("Could not open file \"%s\"\n", filename);
		return false;
	}

	// Get file size (we're already at end due to std::ios::ate)
	std::streamsize size = file.tellg();
	if (size < 0) {
		printf("Could not get file size for \"%s\"\n", filename);
		return false;
	}

	// Go back to beginning
	file.seekg(0, std::ios::beg);

	// Resize vector and read directly into it
	m_RawText.resize(static_cast<size_t>(size));
	if (!file.read(reinterpret_cast<char*>(m_RawText.data()), size)) {
		m_RawText.clear();
		printf("Could not read file \"%s\"\n", filename);
		return false;
	}

	return true;
}



void
LexerBase::displayErrorMessage(const uint8_t *pTokenStart, const uint8_t *pTokenErr, int tokenLineNumber)
{
	const uint8_t *pRawText = m_RawText.data();
	const uint8_t *pRawTextEnd = pRawText + m_RawText.size();

	const uint8_t *p;

	// Find start of line with error
	const uint8_t *pLineStart = pTokenStart;
	for (; pLineStart > pRawText && *(pLineStart - 1) != '\n' && *(pLineStart - 1) != '\r'; --pLineStart);

	// Find end of line with error
	const uint8_t *pLineEnd = pTokenStart;
	for (; pLineEnd < pRawTextEnd && *pLineEnd != '\n' && *pLineEnd != '\r'; ++pLineEnd);

	// Error message
	printf("Error in line %d:\n%s\n", tokenLineNumber, std::string((const char *)pLineStart, pLineEnd - pLineStart).c_str());
	for (p = pLineStart; p < pTokenStart; ++p) printf(" ");
	for (; p < pTokenErr && p < pLineEnd; ++p) printf("^");
	printf("\n");
}


void
LexerBase::debug()
{
	printf("\n");
	printf("Text file translated to tokens\n");
	printf("  n  line  id      TOKEN       \"string\"\n");
	printf("----+----+----+---------------+--------------------\n");

	int n = 0;
	for (auto it = m_TokenSequence.begin(); it != m_TokenSequence.end(); ++it, ++n) {
		std::string lexeme((const char *)&m_RawText[it->lexemeStart], it->lexemeLength);
		printf("%4d %4d %4u %15s  \"%s\"\n", n, it->lineNumber, it->tokenClassId, m_TokenClasses[it->tokenClassId].name.c_str(), lexeme.c_str());
	}
}
