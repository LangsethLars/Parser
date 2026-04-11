#include "ParserBase.h"



ParserBase::ParserBase(
	const VariableClasses &vc,
	const Productions &prod,
	const ParsingTable &pt,
	const TokenClasses &tc,
	const TokenSequence &ts,
	const RawText &rt
) :
	m_VariableClasses(vc),
	m_Productions(prod),
	m_ParsingTable(pt),
	m_TokenClasses(tc),
	m_TokenSequence(ts),
	m_RawText(rt)
{
}



bool ParserBase::parseTokenSequence()
{
	bool ok = true;

	m_ParseTree.clear();

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



int ParserBase::getVariableClassId(const char *name)
{
	for (size_t i = 0; i < m_VariableClasses.size(); ++i)
		if (m_VariableClasses[i].name == name)
			return int(i);

	return -1;
}



void
ParserBase::displayErrorMessage(const unsigned char *pTokenStart, const unsigned char *pTokenErr, int tokenLineNumber)
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
ParserBase::debug()
{
	int n;

	printf("\n");
	printf("Text file translated to tokens\n");
	printf("  n  line  id      TOKEN       \"string\"\n");
	printf("----+----+----+---------------+--------------------\n");
	n = 0;
	for (auto it = m_TokenSequence.begin(); it != m_TokenSequence.end(); ++it, ++n) {
		std::string lexeme((const char *)&m_RawText[it->lexemeStart], it->lexemeLength);
		printf("%4d %4d %4u %15s  \"%s\"\n", n, it->lineNumber, it->tokenClassId, m_TokenClasses[it->tokenClassId].name.c_str(), lexeme.c_str());
	}

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

}
