#include "Generator.h"
#include "ChSet.h"

#include <iterator>



bool Generator::makeCodeFromScript(const char *rootName, bool bOnlyLexer)
{
	clear();
	std::string filename = "scripts/" + std::string(rootName) + ".txt";
	bool ok = m_Parser.lexAndParseFile(filename.c_str());

	if (ok) {
		ok = buildLexer(rootName);
		if (ok && !bOnlyLexer) {
			ok = buildParser(rootName);
		}
	}

	return ok;
}



bool Generator::buildLexer(const char* rootName)
{
	bool ok = true;
	ParseTreeIterator it(m_Parser.m_ParseTree, m_Parser.m_TokenSequence, m_Parser.m_RawText);

	try {

		// Rule <> = <Start> END_OF_FILE;
		if (it.getVariableIdNoThrow() != int(Bootstrap_Parser::VariableId::__)) {
			throw std::runtime_error("Root node of parse tree is not <>.");
		}
		it.moveToFirstChild();

		// Skip <Start> = <Stmts>;
		// Skip <Stmts> = <TokenDecl> <Stmts> | <RuleDecl> <Stmts> | ;
		// Siblings are either <TokenDecl>, <RuleDecl> or END_OF_FILE. We only care about <TokenDecl>.
		for (; it.hasNextSibling(); it.moveToNextSibling()) {

			// Skip other rules than <TokenDecl>
			if (it.getVariableIdNoThrow() != int(Bootstrap_Parser::VariableId::_TokenDecl_)) {
				continue;
			}

			// Rule <TokenDecl>		= <TokenType> LABEL EQUAL <TokenExpr> SEMICOLON;
			// Skip <TokenType>		= _Token | _Temp | _Ignore;
			ParseTreeIterator itTokenDecl = it.firstChild();	// <TokenType> is the first child of <TokenDecl>
			ParseTreeIterator itTokenDeclSemicolon = it.lastChild();	// SEMICOLON is the last child of <TokenDecl>

			// _Token or _Temp or _Ignore
			int tokenType = itTokenDecl.getTokenIdNoThrow();
			if (
				tokenType != int(Bootstrap_Lexer::TokenId::_Token) &&
				tokenType != int(Bootstrap_Lexer::TokenId::_Temp) &&
				tokenType != int(Bootstrap_Lexer::TokenId::_Ignore)
				) {
				throw std::runtime_error("Expected _Token, _Temp or _Ignore in token declaration.");
			}
			itTokenDecl.moveToNextSibling();

			// LABEL
			if (itTokenDecl.getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::LABEL)) {
				throw std::runtime_error("Expected LABEL in token declaration.");
			}
			std::string label = itTokenDecl.getLexeme();
			itTokenDecl.moveToNextSibling();

			// EQUAL
			if (itTokenDecl.getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::EQUAL)) {
				throw std::runtime_error("Expected EQUAL in token declaration.");
			}
			itTokenDecl.moveToNextSibling();

			// Skip <TokenExpr>
			// <TokenExprPar> | LABEL | CONST_CH | CONST_STRING | CONST_SET
			if (itTokenDecl == itTokenDeclSemicolon) {
				throw std::runtime_error("Missing token expression in token declaration.");
			}
			BuildNFA nfa;
			getTokenExpr(itTokenDecl, itTokenDeclSemicolon.prevSibling(), nfa);
			if (tokenType == int(Bootstrap_Lexer::TokenId::_Temp)) {
				if (m_TempNFA.find(label) != m_TempNFA.end()) {
					throw std::runtime_error("Error: Temp label \"" + label + "\" already exist.");
				}
				m_TempNFA[label] = nfa;
			}
			else {
				m_MainNFA.addToken(nfa, label.c_str(), tokenType == int(Bootstrap_Lexer::TokenId::_Ignore));
			}

			// SEMICOLON
			if (itTokenDeclSemicolon.getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::SEMICOLON)) {
				throw std::runtime_error("Expected SEMICOLON at end of token declaration.");
			}

		}

		if (it.getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::END_OF_FILE)) {
			throw std::runtime_error("Expected END_OF_FILE at end of parser tree.");
		}

	}
	catch (const std::exception& ex) {
		it.debug(ex.what());
		return false;
	}

	// Create DFA lookup table for lexer
	m_MainNFA.convertToDFA();

	// Save DFA tables to files that can be used by the lexer.
	ok &= m_MainNFA.saveDFA(rootName);

	return ok;

}



// Skip <TokenExpr>		= <TokenExprPar> <TokenExprTail>
//						| LABEL          <TokenExprTail>
//						| CONST_CH       <TokenExprTail>
//						| CONST_STRING   <TokenExprTail>
//						| CONST_SET      <TokenExprTail>;
// Rule <TokenExprPar>	= OPEN_PAR  <TokenExpr> CLOSE_PAR;
// Skip <TokenExprTail>	= <TokenRepeat> <TokenExprTail>
//						| <TokenExpr>
//						| OR <TokenExpr>
//						| ;
// Skip <TokenRepeat>	= ASTERIX | PLUS | QUESTION;

void Generator::getTokenExpr(ParseTreeIterator it, ParseTreeIterator itLast, BuildNFA &nfa)
{
	BuildNFA nfaTemp;
	bool or = false;

	if ( it.getNodeIndex() > itLast.getNodeIndex() ) {
		throw std::runtime_error("Unexpected end of token expression.");
	}

	while ( it.getNodeIndex() <= itLast.getNodeIndex() ) {

		nfaTemp.clear();

		// <TokenExprPar> | LABEL | CONST_CH | CONST_STRING | CONST_SET

		if (it.getVariableIdNoThrow() == int(Bootstrap_Parser::VariableId::_TokenExprPar_)) {

			// <TokenExprPar>
			if (it.firstChild().getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::OPEN_PAR)) {
				throw std::runtime_error("Expected OPEN_PAR in token expression.");
			}
			if (it.lastChild().getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::CLOSE_PAR)) {
				throw std::runtime_error("Expected CLOSE_PAR in token expression.");
			}
			getTokenExpr(it.firstChild().nextSibling(), it.lastChild().prevSibling(), nfaTemp);

		} else {

			Token token = it.getToken();

			if (token.tokenClassId == int(Bootstrap_Lexer::TokenId::LABEL)) {

				// LABEL
				std::string lexeme = it.getLexeme();
				auto itNFA = m_TempNFA.find(lexeme);
				if (itNFA != m_TempNFA.end()) {
					nfaTemp = itNFA->second;
				} else {
					throw std::runtime_error("Error: Temp label \"" + lexeme + "\" does not exist.");
				}

			} else if (token.tokenClassId == int(Bootstrap_Lexer::TokenId::CONST_CH)) {

				// CONST_CH
				int i = token.lexemeStart + 1;
				nfaTemp.appendCh(getChar(i));

			} else if (token.tokenClassId == int(Bootstrap_Lexer::TokenId::CONST_STRING)) {

				// CONST_STRING
				int i = token.lexemeStart + 1;
				for (; m_Parser.m_RawText[i] != '"';) {
					nfaTemp.appendCh(getChar(i)); // i will be increased
				}

			} else if (token.tokenClassId == int(Bootstrap_Lexer::TokenId::CONST_SET)) {

				// CONST_SET
				ChSet chSet;
				unsigned char ch1, ch2;
				int i = token.lexemeStart + 1;
				for (; m_Parser.m_RawText[i] != ']' && m_Parser.m_RawText[i] != '~';) {
					ch1 = getChar(i);
					if (m_Parser.m_RawText[i] == '-') {
						++i;
						ch2 = getChar(i);
						chSet.addRange(ch1, ch2);
					}
					else {
						chSet.addChar(ch1);
					}
				}
				if (m_Parser.m_RawText[i] == '~') {
					++i;
					for (; m_Parser.m_RawText[i] != ']';) {
						ch1 = getChar(i);
						if (m_Parser.m_RawText[i] == '-') {
							++i;
							ch2 = getChar(i);
							chSet.removeRange(ch1, ch2);
						} else {
							chSet.removeChar(ch1);
						}
					}
				}
				nfaTemp.appendChSet(chSet);

			} else {

				// Unknown token in expression
				std::string lexeme = it.getLexeme();
				throw std::runtime_error("Error: Unknown token \"" + lexeme + "\" in expression.");

			}

		}

		// ASTERIX | PLUS | QUESTION
		for (++it; it.getNodeIndex() <= itLast.getNodeIndex(); ++it) {
			if (it.getTokenIdNoThrow() == int(Bootstrap_Lexer::TokenId::ASTERIX)) {
				nfaTemp.timesZeroOrMore();
			} else if (it.getTokenIdNoThrow() == int(Bootstrap_Lexer::TokenId::PLUS)) {
				nfaTemp.timesOneOrMore();
			} else if (it.getTokenIdNoThrow() == int(Bootstrap_Lexer::TokenId::QUESTION)) {
				nfaTemp.timesZeroOrOne();
			} else {
				break;
			}
		}

		if (or ) {
			nfa.this_or(nfaTemp);
		} else {
			nfa.appendNFA(nfaTemp);
		}

		// OR
		if (it.getTokenIdNoThrow() == int(Bootstrap_Lexer::TokenId::OR)) {
			or = true;
			++it;
		} else {
			or = false;
		}

	}

}



static bool IsHex[256] = {
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	true , true , true , true , true , true , true , true , true , true , false, false, false, false, false, false, // '0' - '9'
	false, true , true , true , true , true , true , false, false, false, false, false, false, false, false, false, // 'A' - 'F'
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, true , true , true , true , true , true , false, false, false, false, false, false, false, false, false, // 'a' - 'f'
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
};



static unsigned HexToInt[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0, // '0' - '9'
	0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 'A' - 'F'
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 'a' - 'f'
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};



unsigned char Generator::getChar(int &i)
{
	unsigned char ch;

	if (m_Parser.m_RawText[i] == '\\') {
		++i;
		if (IsHex[m_Parser.m_RawText[i]]) {
			ch = HexToInt[m_Parser.m_RawText[i++]] << 4;
			ch += HexToInt[m_Parser.m_RawText[i++]];
		}
		else {
			ch = m_Parser.m_RawText[i++];
		}
	}
	else {
		ch = m_Parser.m_RawText[i++];
	}

	return ch;
}



bool Generator::buildParser(const char* rootName)
{
	bool ok = true;
	ParseTreeIterator it(m_Parser.m_ParseTree, m_Parser.m_TokenSequence, m_Parser.m_RawText);

	try {

		// Rule <> = <Start> END_OF_FILE;
		if (it.getVariableIdNoThrow() != int(Bootstrap_Parser::VariableId::__)) {
			throw std::runtime_error("Root node of parse tree is not <>.");
		}
		it.moveToFirstChild();

		// Rule <> that calls <Start>
		m_MainCFG.addProduction("<>").addVariable("<Start>").addTerminal("END_OF_FILE");

		// Skip <Start> = <Stmts>;
		// Skip <Stmts> = <TokenDecl> <Stmts> | <RuleDecl> <Stmts> | ;
		// Siblings are either <TokenDecl>, <RuleDecl> or END_OF_FILE. We only care about <RuleDecl>.
		for (; it.hasNextSibling(); it.moveToNextSibling()) {

			// Skip other rules than <RuleDecl>
			if (it.getVariableIdNoThrow() != int(Bootstrap_Parser::VariableId::_RuleDecl_)) {
				continue;
			}

			// Rule <RuleDecl>     = <RuleType> VARIABLE EQUAL <RuleExpr> SEMICOLON;
			// Skip <RuleType>     = _Rule | _Skip;
			ParseTreeIterator itRuleDecl = it.firstChild();	// <RuleType> is the first child of <RuleDecl>
			ParseTreeIterator itRuleDeclSemicolon = it.lastChild();	// SEMICOLON is the last child of <RuleDecl>

			// _Rule or _Skip
			int ruleType = itRuleDecl.getTokenIdNoThrow();
			if (
				ruleType != int(Bootstrap_Lexer::TokenId::_Rule) &&
				ruleType != int(Bootstrap_Lexer::TokenId::_Skip)
				) {
				throw std::runtime_error("Expected _Rule or _Skip in rule declaration.");
			}
			itRuleDecl.moveToNextSibling();

			// VARIABLE
			if (itRuleDecl.getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::VARIABLE)) {
				throw std::runtime_error("Expected VARIABLE in rule declaration.");
			}
			std::string variable = itRuleDecl.getLexeme();
			itRuleDecl.moveToNextSibling();

			// EQUAL
			if (itRuleDecl.getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::EQUAL)) {
				throw std::runtime_error("Expected EQUAL in rule declaration.");
			}
			itRuleDecl.moveToNextSibling();

			// Skip <RuleExpr> = <RuleProd> (OR <RuleProd>)* SEMICOLON;
			// Rule <RuleProd> = (LABEL | VARIABLE)+
			while (true) {

				// <RuleProd>
				if (itRuleDecl.getVariableIdNoThrow() != int(Bootstrap_Parser::VariableId::_RuleProd_)) {
					throw std::runtime_error("Expected <RuleProd> in rule declaration.");
				}

				BuildCFG& production = m_MainCFG.addProduction(variable.c_str());

				if (itRuleDecl.hasChildren()) {

					ParseTreeIterator itProd = itRuleDecl.firstChild();
					ParseTreeIterator itProdLast = itRuleDecl.lastChild();

					while (true) {

						int tokenId = itProd.getTokenIdNoThrow();

						if (tokenId == int(Bootstrap_Lexer::TokenId::LABEL))
							production.addTerminal(itProd.getLexeme().c_str());
						else if (tokenId == int(Bootstrap_Lexer::TokenId::VARIABLE))
							production.addVariable(itProd.getLexeme().c_str());
						else
							throw std::runtime_error("Expected LABEL or VARIABLE in <RuleProd>");

						if (itProd.hasNextSibling())
							++itProd;
						else
							break;

					}

				}

				if (ruleType == int(Bootstrap_Lexer::TokenId::_Skip)) {
					if (!m_MainCFG.setVariableClassToSkip(variable.c_str())) {
						throw std::runtime_error("Failed to set variable class " + variable + "	to skip.");
					}
				}

				// OR or SEMICOLON
				itRuleDecl.moveToNextSibling();
				if (itRuleDecl.getTokenIdNoThrow() == int(Bootstrap_Lexer::TokenId::OR)) {
					itRuleDecl.moveToNextSibling();
				}
				else if (itRuleDecl.getTokenIdNoThrow() == int(Bootstrap_Lexer::TokenId::SEMICOLON)) {
					break;
				}
				else {
					throw std::runtime_error("Expected OR or SEMICOLON in <RuleExpr>");
				}
			}

		}

		if (it.getTokenIdNoThrow() != int(Bootstrap_Lexer::TokenId::END_OF_FILE)) {
			throw std::runtime_error("Expected END_OF_FILE at end of parser tree.");
		}

	}
	catch (const std::exception& ex) {
		it.debug(ex.what());
		return false;
	}

	// Create DFA lookup table for lexer
	ok = m_MainCFG.isOk() && m_MainCFG.buildParseTable();

	// Save parse tables to files that can be used by the parser.
	m_MainCFG.saveCFG(rootName);

	return ok;

}
