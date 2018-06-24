#include "Parser.h"



Parser::Parser(const TokenClasses &tc, const VariableClasses &vc, const LexerTable &lt, const Productions &prod, const ParsingTable &pt) :
	m_TokenClasses(tc),
	m_VariableClasses(vc),
	m_LexerTable(lt),
	m_Productions(prod),
	m_ParsingTable(pt)
{
}



bool Parser::lexRawText()
{
	const unsigned char *pRawText = (const unsigned char *)&m_RawText[0];
	const unsigned char *p = pRawText;
	const unsigned char *pEnd = pRawText + m_RawText.size();

	int lineNumber = 1;
	bool bNL = false;

	Token token;
	m_TokenSequence.clear();

	while (p < pEnd) {

		int stateNo = 0;
		int tokenId = -1;
		int tokenLineNumber = lineNumber;

		const unsigned char *pTokenStart = p;
		const unsigned char *pTokenEnd = p;
		const unsigned char *pTokenErr = p;

		while (p < pEnd) {

			if ((stateNo = m_LexerTable[stateNo].onChMove[*p++]) < 0) {
				pTokenErr = p;
				p = pTokenEnd;
				break;
			}

			if (m_LexerTable[stateNo].tokenClassId > 0) {
				pTokenEnd = p;
				tokenId = m_LexerTable[stateNo].tokenClassId;
			}

			// For debugging, just handle different forms of NL
			if (p < pEnd) {
				if (bNL)
					++lineNumber, bNL = false;
				else if ((*(p - 1) == 10 && *p == 13) || (*(p - 1) == 13 && *p == 10))
					bNL = true;
				else if (*(p - 1) == 10 || *(p - 1) == 13)
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
//			token.string = std::string((const char *)pTokenStart, pTokenEnd - pTokenStart);
			m_TokenSequence.push_back(token);
		}

	}

	// End with and End Of File token (id = 0)
	token.tokenClassId = 0;
	token.lineNumber = lineNumber;
	token.lexemeStart = 0; // Could have used m_RawText.size();
	token.lexemeLength = 0;
	m_TokenSequence.push_back(token);

	return true;

}



bool Parser::lexAndParseRawText()
{
	m_ParseTree.clear();
	bool ok = lexRawText();

	int nOfVar = m_VariableClasses.size();
	int nOfTok = m_TokenClasses.size();

	int tokenNo = 0;
	int tokenId;

	int productionNo;

	std::vector<int> stack;
	int symb = -1; // Variable number 0, <>

	std::vector<int> parent;
	int currentParent = -1; // No parent yet
	int lastChild = -1; // No child yet
	int current = -1; // No current yet

	stack.emplace_back(symb);
	parent.emplace_back(currentParent);


	while (ok && stack.size() > 0) {

		symb = stack.back();
		if (tokenNo >= int(m_TokenSequence.size())) {
			printf("Error in parsing: No more tokens. Next symbol is %s\n", symb < 0 ? m_VariableClasses[-(symb+1)].name.c_str() : m_TokenClasses[symb].name.c_str());
			ok = false;
			break;
		}
		tokenId = m_TokenSequence[tokenNo].tokenClassId;

		if (symb == tokenId) {

			// std::string lexeme((const char *)&m_RawText[m_TokenSequence[tokenNo].lexemeStart], m_TokenSequence[tokenNo].lexemeLength);
			// printf("Parsing token match: %42s    \"%20s\"\n", m_TokenClasses[tokenId].name.c_str(), lexeme.c_str());
			
			current = int(m_ParseTree.size());
			currentParent = parent.back();
			lastChild = m_ParseTree[currentParent].lastChild;

			m_ParseTree.emplace_back();
			m_ParseTree.back().symbolIdOrTokenSequenceNo = tokenNo;
			m_ParseTree.back().parent = currentParent;
			m_ParseTree.back().firstChild = -1;
			m_ParseTree.back().lastChild = -1;
			m_ParseTree.back().prev = lastChild;
			m_ParseTree.back().next = -1;

			if (lastChild >= 0) {
				m_ParseTree[lastChild].next = current;
			}

			if (m_ParseTree[currentParent].firstChild < 0) {
				m_ParseTree[currentParent].firstChild = current;
			}

			m_ParseTree[currentParent].lastChild = current;

			stack.pop_back();
			parent.pop_back();
			++tokenNo;

		} else if (symb >= 0) {

			printf("Error in parsing: Token %d with id %d should have been %d.\n", tokenNo, tokenId, symb);
			displayErrorMessage(&m_RawText[m_TokenSequence[tokenNo].lexemeStart], &m_RawText[m_TokenSequence[tokenNo].lexemeStart + m_TokenSequence[tokenNo].lexemeLength], m_TokenSequence[tokenNo].lineNumber);

			ok = false;
			break;

		} else {

			int varId = -(symb + 1);
			productionNo = m_ParsingTable[varId*nOfTok + tokenId];

			if (productionNo < 0) {

				printf("Error in parsing: No matching production for variable %s and token %s\n", m_VariableClasses[varId].name.c_str(), m_TokenClasses[tokenId].name.c_str());
				displayErrorMessage(&m_RawText[m_TokenSequence[tokenNo].lexemeStart], &m_RawText[m_TokenSequence[tokenNo].lexemeStart + m_TokenSequence[tokenNo].lexemeLength], m_TokenSequence[tokenNo].lineNumber);
				ok = false;
				break;

			} else {

				// printf("Parsing production match: Table[%15s,%15s] ==> %d\n", m_VariableClasses[varId].name.c_str(), m_TokenClasses[tokenId].name.c_str(), productionNo);

				current = int(m_ParseTree.size());
				currentParent = parent.back();
				lastChild = currentParent >= 0 ? m_ParseTree[currentParent].lastChild : -1;

				if (!m_VariableClasses[varId].skip) {

					m_ParseTree.emplace_back();
					m_ParseTree.back().symbolIdOrTokenSequenceNo = symb;
					m_ParseTree.back().parent = currentParent;
					m_ParseTree.back().firstChild = -1;
					m_ParseTree.back().lastChild = -1;
					m_ParseTree.back().prev = lastChild;
					m_ParseTree.back().next = -1;

					if (lastChild >= 0) {
						m_ParseTree[lastChild].next = current;
					}
					
					if (currentParent >= 0 ) {
						if (m_ParseTree[currentParent].firstChild < 0) {
							m_ParseTree[currentParent].firstChild = current;
						}
						m_ParseTree[currentParent].lastChild = current;
					}

					currentParent = current;
				}

				stack.pop_back();
				parent.pop_back();
				for (auto it = m_Productions[productionNo].symbols.rbegin(); it != m_Productions[productionNo].symbols.rend(); ++it) {
					stack.emplace_back(*it);
					parent.emplace_back(currentParent);
				}

			}

		}
		
	}

	if (ok) {
		if (stack.size() > 0) {
			printf("Error in parsing: %d symbols left on stack, nest symbol is %s\n", int(stack.size()), stack.back() < 0 ? m_VariableClasses[-(stack.back() + 1)].name.c_str() : m_TokenClasses[stack.back()].name.c_str());
			ok = false;
		}
		if (tokenNo < int(m_TokenSequence.size())) {
			printf("Error in parsing: %d tokens left in sequence, nest token is %s\n", int(m_TokenSequence.size()) - tokenNo, m_TokenClasses[tokenNo].name.c_str());
			displayErrorMessage(&m_RawText[m_TokenSequence[tokenNo].lexemeStart], &m_RawText[m_TokenSequence[tokenNo].lexemeStart + m_TokenSequence[tokenNo].lexemeLength], m_TokenSequence[tokenNo].lineNumber);
			ok = false;
		}
	}

	return ok;
}



int Parser::getTokenClassId(const char *name)
{
	for (int i = 0; i < m_TokenClasses.size(); ++i)
		if (m_TokenClasses[i].name == name)
			return i;

	return -1;
}



int Parser::getVariableClassId(const char *name)
{
	for (int i = 0; i < m_VariableClasses.size(); ++i)
		if (m_VariableClasses[i].name == name)
			return i;

	return -1;
}



void
Parser::displayErrorMessage(const unsigned char *pTokenStart, const unsigned char *pTokenErr, int tokenLineNumber)
{
	const unsigned char *pRawText = (const unsigned char *)&m_RawText[0];
	const unsigned char *pRawTextEnd = pRawText + m_RawText.size();

	const unsigned char *p;

	// Find start of line with error
	const unsigned char *pLineStart = pTokenStart;
	for (; pLineStart >= pRawText && *pLineStart != 10 && *pLineStart != 13; --pLineStart); ++pLineStart;

	// Find end of line with error
	const unsigned char *pLineEnd = pTokenStart;
	for (; pLineEnd < pRawTextEnd && *pLineEnd != 10 && *pLineEnd != 13; ++pLineEnd);

	// Error message
	printf("Error in line %d:\n%s\n", tokenLineNumber, std::string((const char *)pLineStart, pLineEnd - pLineStart).c_str());
	for (p = pLineStart; p < pTokenStart; ++p) printf(" ");
	for (; p < pTokenErr && p < pLineEnd; ++p) printf("^");
	printf("\n");

}	// End of displayErrorMessage()


void
Parser::debug()
{
	int n;
	/**/
	printf("\n");
	printf("Text file translated to tokens\n");
	printf("  n  line  id      TOKEN       \"string\"\n");
	printf("----+----+----+---------------+--------------------\n");
	n = 0;
	for (auto it = m_TokenSequence.begin(); it != m_TokenSequence.end(); ++it, ++n) {
		std::string lexeme((const char *)&m_RawText[it->lexemeStart], it->lexemeLength);
		printf("%4d %4d %4u %15s  \"%s\"\n", n, it->lineNumber, it->tokenClassId, m_TokenClasses[it->tokenClassId].name.c_str(), lexeme.c_str());
	}
	/**/

	/**/
	printf("\n");
	printf("Parse tree:\n");
	printf("  n          symbol(nnn) parent fChild lChild next prev string\n");
	printf("----+--------------------+-----+-----+-----+-----+----------------------\n");
	n = 0;
	for (auto it = m_ParseTree.begin(); it != m_ParseTree.end(); ++it, ++n) {
		std::string lexeme;
		if (it->symbolIdOrTokenSequenceNo >= 0) lexeme = std::string((const char *)&m_RawText[m_TokenSequence[it->symbolIdOrTokenSequenceNo].lexemeStart], m_TokenSequence[it->symbolIdOrTokenSequenceNo].lexemeLength);
		printf("%4d %15s(%3d) %5d %5d %5d %5d %5d %s\n", n, it->symbolIdOrTokenSequenceNo >= 0 ? m_TokenClasses[m_TokenSequence[it->symbolIdOrTokenSequenceNo].tokenClassId].name.c_str() : m_VariableClasses[-(it->symbolIdOrTokenSequenceNo + 1)].name.c_str(), it->symbolIdOrTokenSequenceNo, it->parent, it->firstChild, it->lastChild, it->next, it->prev, lexeme.c_str());
	}
	/**/

	/**/
	printf("\n");
	printf("Parse tree:\n");
	printf("-----------\n");
	int current = 0;
	int level = 0;
	bool more = m_ParseTree.size() > 0;
	while (more) {
		printf("%3d:", current);
		for (int i = 0; i < level; ++i)
			printf("    ");
		if (m_ParseTree[current].symbolIdOrTokenSequenceNo >= 0) {
			std::string lexeme((const char *)&m_RawText[m_TokenSequence[m_ParseTree[current].symbolIdOrTokenSequenceNo].lexemeStart], m_TokenSequence[m_ParseTree[current].symbolIdOrTokenSequenceNo].lexemeLength);
			printf("%s(\"%s\")\n", m_TokenClasses[m_TokenSequence[m_ParseTree[current].symbolIdOrTokenSequenceNo].tokenClassId].name.c_str(), lexeme.c_str());
			if (m_TokenSequence[m_ParseTree[current].symbolIdOrTokenSequenceNo].tokenClassId == 0)
				break;
			while ((n = m_ParseTree[current].next) < 0) {
				current = m_ParseTree[current].parent;
				--level;
			}
			current = n;
		} else {
			printf("%s\n", m_VariableClasses[-(m_ParseTree[current].symbolIdOrTokenSequenceNo + 1)].name.c_str());
			if ((n = m_ParseTree[current].firstChild) < 0) {
				while ((n = m_ParseTree[current].next) < 0) {
					current = m_ParseTree[current].parent;
					--level;
				}
				--level;
			}
			current = n;
			++level;
		}
	}
	/**/

}



void NodeIter::debug(const char *msg)
{
	printf("NodeIter message: %s\n    m_NTokenSequence = %d\n", msg, m_NParseTree);
	if (m_ParserPtr != 0 && m_NParseTree >= 0 && m_NParseTree < m_ParserPtr->m_ParseTree.size()) {
		ParseNode &node = m_ParserPtr->m_ParseTree[m_NParseTree];
		printf("    Node: symb=%d, parent=%d, firstChild = %d, lastChild=%d, prev=%d, next =%d\n", node.symbolIdOrTokenSequenceNo, node.parent, node.firstChild, node.lastChild, node.prev, node.next);
		if (node.symbolIdOrTokenSequenceNo < 0) {
			int ind = -(node.symbolIdOrTokenSequenceNo + 1);
			if (ind >= 0 && ind < m_ParserPtr->m_VariableClasses.size())
				printf("    Symbol = %s\n", m_ParserPtr->m_VariableClasses[ind].name.c_str());
			else
				printf("    Symbol out of range.\n");
		} else {
			if (node.symbolIdOrTokenSequenceNo < m_ParserPtr->m_TokenSequence.size()) {
				Token &token = getToken();
				printf("    Token: id=%d, line=%d, start=%d, length=%d\n", token.tokenClassId, token.lineNumber, token.lexemeStart, token.lexemeLength);
				if (token.tokenClassId >= 0 && token.tokenClassId < m_ParserPtr->m_TokenClasses.size()) {
					std::string str = lexeme();
					printf("        Token %s = \"%s\"\n", m_ParserPtr->m_TokenClasses[token.tokenClassId].name.c_str(), str.c_str());
					const unsigned char *p = &m_ParserPtr->m_RawText[token.lexemeStart];
					m_ParserPtr->displayErrorMessage(p, p + token.lexemeLength, token.lineNumber);
				} else {
					printf("        Internal error in Token.\n");
				}
			} else {
				printf("    Internal error in Node, symb out of range.\n");
			}
		}
	} else {
		printf("    Internal error in NodeIter.\n");
	}
}
