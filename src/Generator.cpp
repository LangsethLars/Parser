#include "Generator.h"
#include "ChSet.h"
#include "Parser_Bootstrap.h"

#include <iterator>

using namespace Parser_Bootstrap;



bool Generator::bootstrap()
{
	clear();

	bool ok =
		bootstrapLexer() &&
		bootstrapParser() &&
		saveTables("Bootstrap");

	return ok;
}



bool Generator::buildTablesFrom(char *rootName)
{
	clear();

	bool ok =
		readCfgFile(rootName, m_Parser.m_RawText) &&
		m_Parser.lexAndParseRawText() &&
		buildLexer() &&
		buildParser() &&
		saveTables(rootName);

	//m_Parser.debug();
	return ok;
}



bool Generator::buildLexer()
{
	bool ok = true;

	NodeIter head(m_Parser);
	NodeIter decl = head.firstChild();
	NodeIter eof = head.lastChild();

	for (; decl != eof; ++decl) {

		// <TokenDecl>  = <TokenType> LABEL EQUAL <TokenExpr> SEMICOLON
		if (decl.symbol() == int(SymbolId::_TokenDecl_)) {

			// _Token or _Temp or _Ignore
			NodeIter it = decl.firstChild();
			bool tok  = it.symbol() == int(TokenId::_Token);
			bool temp = it.symbol() == int(TokenId::_Temp);
			bool ignore = it.symbol() == int(TokenId::_Ignore);

			// LABEL
			++it;
			std::string label = it.lexeme();

			// EQUAL
			++it;

			// <TokenExpr>
			++it;
			BuildNFA nfa;
			ok &= getTokenExpr(it, decl.lastChild(), nfa);

			if (temp) {
				if (m_TempNFA.find(label) != m_TempNFA.end()) {
					printf("Error: Temp label \"%s\" already exist.\n", label.c_str());
					m_StatusOk = ok = false;
				}
				m_TempNFA[label] = nfa;
			} else {
				addNFA(nfa, label.c_str(), ignore);
			}

		} // else ignore

	}

	// Create DFA lookup table for lexer
	m_MainNFA.convertToDFA(m_LexerTable);

	return ok;

}



bool Generator::getTokenExpr(NodeIter it, NodeIter itEnd, BuildNFA &nfa)
{
	bool ok = true;

	//it.debug("Debug it");
	//itEnd.debug("Debug itEnd");

	BuildNFA nfaTemp;
	bool or = false;
	while ( it != itEnd ) {

		//it.debug("Debug");

		nfaTemp.clear();

		// <TokenExprPar> | LABEL | CONST_CH | CONST_STRING | CONST_SET
		if (it.symbol() == int(SymbolId::_TokenExprPar_)) {

			getTokenExpr(it.firstChild().next(), it.lastChild(),nfaTemp);

		} else {

			Token &token = it.getToken();

			if (token.tokenClassId == int(TokenId::LABEL)) {

				std::string lexeme((const char *)&m_Parser.m_RawText[token.lexemeStart], token.lexemeLength);
				auto itNFA = m_TempNFA.find(lexeme);
				if (itNFA != m_TempNFA.end()) {
					nfaTemp = itNFA->second;
				} else {
					it.debug("Missing temp label");
				}

			} else if (token.tokenClassId == int(TokenId::CONST_CH)) {

				int i = token.lexemeStart + 1;
				nfaTemp.appendCh(getChar(i));

			} else if (token.tokenClassId == int(TokenId::CONST_STRING)) {

				int i = token.lexemeStart + 1;
				for (; m_Parser.m_RawText[i] != '"';) {
					nfaTemp.appendCh(getChar(i)); // i will be increased
				}

			} else if (token.tokenClassId == int(TokenId::CONST_SET)) {

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

				//ok = false;
				it.debug("Unknown TokenId in expression");
				//debug("Unknown TokenId in expression", it.getN());

			}

		}

		// ASTERIX | PLUS | QUESTION
		for (++it; it != itEnd; ++it) {
			if (it.symbol() == int(TokenId::ASTERIX)) {
				nfaTemp.timesZeroOrMore();
			} else if (it.symbol() == int(TokenId::PLUS)) {
				nfaTemp.timesOneOrMore();
			} else if (it.symbol() == int(TokenId::QUESTION)) {
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
		if (it.symbol() == int(TokenId::OR)) {
			or = true;
			++it;
		} else {
			or = false;
		}

	}

	return true;

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



bool Generator::buildParser()
{
	bool ok = true;

	NodeIter head(m_Parser);
	NodeIter decl = head.firstChild();
	NodeIter eof = head.lastChild();

	// Rule <> that calls <Start>
	m_ParserDef.addProduction("<>").addVariable("<Start>").addTerminal("END_OF_FILE");

	for (; decl != eof; ++decl) {

		// Rule <RuleDecl> = <RuleType> VARIABLE EQUAL <RuleExpr> SEMICOLON
		if (decl.symbol() == int(SymbolId::_RuleDecl_)) {

			// _Rule or _Skip
			NodeIter it = decl.firstChild();
			bool isRule = it.symbol() == int(TokenId::_Rule);
			bool isSkip = it.symbol() == int(TokenId::_Skip);

			// VARIABLE
			++it;
			std::string variable = it.lexeme();
			int vId = it.symbol();

			// EQUAL
			++it;

			// <RuleExpr> is <RuleProd> (OR <RuleProd)* SEMICOLON
			++it;
			while (it.symbol() == int(SymbolId::_RuleProd_)) {

				BuildParser &production = m_ParserDef.addProduction(variable.c_str());
				if (it.hasChildren()) {
					NodeIter prodIter = it.firstChild();
					NodeIter prodIterEnd = it.lastChild();
					for (;;) {
						int s = prodIter.symbol();
						if (s == int(TokenId::LABEL))
							production.addTerminal(prodIter.lexeme().c_str());
						else if (s == int(TokenId::VARIABLE))
							production.addVariable(prodIter.lexeme().c_str());
						else
							prodIter.debug("Expected LABEL or VARIABLE in <RuleProd>");
						if (prodIter == prodIterEnd)
							break;
						++prodIter;
					}
				}

				// OR or SEMICOLON
				++it;
				if (it.symbol() == int(TokenId::OR))
					++it;
				else if (it.symbol() == int(TokenId::SEMICOLON))
					break;
				else
					it.debug("Expected OR or SEMICOLON in <RuleProd>");

			}
			if (isSkip)
				setVariableClassToSkip(variable.c_str());

		} // else ignore

	}

	// Create DFA lookup table for lexer
	ok = ok && m_ParserDef.isOk() && m_ParserDef.buildParceTable(m_ParsingTable);

	return ok;

}



bool Generator::bootstrapLexer()
{
	BuildNFA nfa1, nfa2;
	bool ignore = true;

	// ChSet .  = [\01-\FF~\0A\0D]; # Often predefined, all characters except \00, lf and cr.
	ChSet chSet_All;
	chSet_All.addRange(1, 255);
	chSet_All.removeChar(10);
	chSet_All.removeChar(13);

	// ChSet sp = [\09\20]; # Space characters, tab and space. Could have ignored more space characters.
	ChSet chSet_Sp;
	chSet_Sp.addChar(9);
	chSet_Sp.addChar(32);

	// Ignore COMMENT = '#' .*;
	nfa1.clear().appendCh('#');
	nfa2.clear().appendChSet(chSet_All).timesZeroOrMore();
	nfa1.appendNFA(nfa2);
	addNFA(nfa1, "COMMENT", ignore);

	// Ignore SPACE = sp+;
	nfa1.clear().appendChSet(chSet_Sp).timesOneOrMore();
	addNFA(nfa1, "SPACE", ignore);

	// Ignore NL = '\0A' | '\0D' | "\0D\0A" | "\0A\0D";  # PS: Maximum length match, will cover most cases.
	nfa1.clear().appendCh(10);
	nfa2.clear().appendCh(13);
	nfa1.this_or(nfa2);
	nfa2.clear().appendCh(13).appendCh(10);
	nfa1.this_or(nfa2);
	nfa2.clear().appendCh(10).appendCh(13);
	nfa1.this_or(nfa2);
	addNFA(nfa1, "NL", ignore);

	// ChSet h = [0-9A-Fa-f]; # Hexadecimal digits.
	ChSet chSet_Hex;
	chSet_Hex.addRange('0', '9');
	chSet_Hex.addRange('A', 'F');
	chSet_Hex.addRange('a', 'f');

	// ChSet g = [!"#$%&'()*+,\-./:;<=>?@[\\\]^_`{|}\~]; # Visible graphical characters (not alphabetic or digit).
	ChSet chSet_Graphical;
	chSet_Graphical.addChars("!\"#$%&'()*+,-./:;<=>?@[\\]^_'{|}~");

	// Temp ESC_CH_G = '\\' g;
	BuildNFA nfa_ESC_CH_G;
	nfa_ESC_CH_G.appendCh('\\');
	nfa_ESC_CH_G.appendChSet(chSet_Graphical);

	// Temp ESC_CH_HEX = '\\' h h;
	BuildNFA nfa_ESC_CH_HEX;
	nfa_ESC_CH_HEX.appendCh('\\');
	nfa_ESC_CH_HEX.appendChSet(chSet_Hex);
	nfa_ESC_CH_HEX.appendChSet(chSet_Hex);

	// Temp ESC_CH = ESC_CH_HEX | ESC_CH_G;
	BuildNFA nfa_ESC_CH;
	nfa_ESC_CH.appendNFA(nfa_ESC_CH_G);
	nfa_ESC_CH.this_or(nfa_ESC_CH_HEX);

	// Temp CONST_SET_CH = [\01-\FF~\0A\0D\-\\\]\~] | ESC_CH; # Excluded lf cr - \ ] ~
	ChSet chSet_ConstSetCh;
	chSet_ConstSetCh.addRange(1, 255);
	chSet_ConstSetCh.removeChar(10);
	chSet_ConstSetCh.removeChar(13);
	chSet_ConstSetCh.removeChar('-');
	chSet_ConstSetCh.removeChar('\\');
	chSet_ConstSetCh.removeChar(']');
	chSet_ConstSetCh.removeChar('~');
	BuildNFA nfa_CONST_SET_CH;
	nfa_CONST_SET_CH.appendChSet(chSet_ConstSetCh);
	nfa_CONST_SET_CH.this_or(nfa_ESC_CH);

	// Temp CONST_SET_CHARS = ((CONST_SET_CH '-' CONST_SET_CH) | CONST_SET_CH)*;
	BuildNFA nfa_CONST_SET_CHARS;
	nfa_CONST_SET_CHARS.appendNFA(nfa_CONST_SET_CH);
	nfa_CONST_SET_CHARS.appendCh('-');
	nfa_CONST_SET_CHARS.appendNFA(nfa_CONST_SET_CH);
	nfa_CONST_SET_CHARS.this_or(nfa_CONST_SET_CH);
	nfa_CONST_SET_CHARS.timesZeroOrMore();

	// Token CONST_CH = '\'' ([\01-\FF~\0A\0D'\\]  | ESC_CH)  '\'';
	ChSet chSet_ConstChCh;
	chSet_ConstChCh.addRange(1, 255);
	chSet_ConstChCh.removeChar(10);
	chSet_ConstChCh.removeChar(13);
	chSet_ConstChCh.removeChar('\'');
	chSet_ConstChCh.removeChar('\\');
	nfa2.clear().appendChSet(chSet_ConstChCh).this_or(nfa_ESC_CH);
	nfa1.clear().appendCh('\'').appendNFA(nfa2).appendCh('\'');
	addNFA(nfa1, "CONST_CH");

	// Token CONST_STRING = '"' ([\01-\FF~\0A\0D"\\] | ESC_CH)* '"';
	ChSet chSet_ConstStringCh;
	chSet_ConstStringCh.addRange(1, 255);
	chSet_ConstStringCh.removeChar(10);
	chSet_ConstStringCh.removeChar(13);
	chSet_ConstStringCh.removeChar('"');
	chSet_ConstStringCh.removeChar('\\');
	nfa2.clear().appendChSet(chSet_ConstStringCh).this_or(nfa_ESC_CH).timesZeroOrMore();
	nfa1.clear().appendCh('"').appendNFA(nfa2).appendCh('"');
	addNFA(nfa1, "CONST_STRING");

	// Token CONST_SET = '[' CONST_SET_CHARS ('~' CONST_SET_CHARS)? ']';
	nfa2.clear().appendCh('~').appendNFA(nfa_CONST_SET_CHARS).timesZeroOrOne();
	nfa1.clear().appendCh('[').appendNFA(nfa_CONST_SET_CHARS).appendNFA(nfa2).appendCh(']');
	addNFA(nfa1, "CONST_SET");

	ChSet chSet_Alpha_;
	chSet_Alpha_.addRange('A', 'Z');
	chSet_Alpha_.addRange('a', 'z');
	chSet_Alpha_.addChar('_');

	ChSet chSet_AlphaNum_;
	chSet_AlphaNum_.addRange('A', 'Z');
	chSet_AlphaNum_.addRange('a', 'z');
	chSet_AlphaNum_.addRange('0', '9');
	chSet_AlphaNum_.addChar('_');

	// Token LABEL = [_A-Za-z] [_A-Za-z0-9]*;
	nfa2.clear().appendChSet(chSet_AlphaNum_).timesZeroOrMore();
	nfa1.clear().appendChSet(chSet_Alpha_).appendNFA(nfa2);
	addNFA(nfa1, "LABEL");

	// Token VARIABLE = '<' [_A-Za-z] [_A-Za-z0-9]* '>';
	nfa2.clear().appendChSet(chSet_AlphaNum_).timesZeroOrMore();
	nfa1.clear().appendCh('<').appendChSet(chSet_Alpha_).appendNFA(nfa2).appendCh('>');
	addNFA(nfa1, "VARIABLE");

	// Token _Token = "Token";
	nfa1.clear().appendStr("Token");
	addNFA(nfa1, "_Token");

	// Token _Temp = "Temp";
	nfa1.clear().appendStr("Temp");
	addNFA(nfa1, "_Temp");

	// Token _Ignore = "Ignore";
	nfa1.clear().appendStr("Ignore");
	addNFA(nfa1, "_Ignore");

	// Token _Rule_ = "Rule";
	nfa1.clear().appendStr("Rule");
	addNFA(nfa1, "_Rule");

	// Token _Skip_ = "Skip";
	nfa1.clear().appendStr("Skip");
	addNFA(nfa1, "_Skip");

	// Token EQUAL = '=';
	nfa1.clear().appendCh('=');
	addNFA(nfa1, "EQUAL");

	// Token OPEN_PAR = '(';
	nfa1.clear().appendCh('(');
	addNFA(nfa1, "OPEN_PAR");

	// Token CLOSE_PAR = ')';
	nfa1.clear().appendCh(')');
	addNFA(nfa1, "CLOSE_PAR");

	// Token ASTERIX = '*';
	nfa1.clear().appendCh('*');
	addNFA(nfa1, "ASTERIX");

	// Token PLUS = '+';
	nfa1.clear().appendCh('+');
	addNFA(nfa1, "PLUS");

	// Token QUESTION = '?';
	nfa1.clear().appendCh('?');
	addNFA(nfa1, "QUESTION");

	// Token OR = '|';
	nfa1.clear().appendCh('|');
	addNFA(nfa1, "OR");

	// Token SEMICOLON = ';';
	nfa1.clear().appendCh(';');
	addNFA(nfa1, "SEMICOLON");

	// Debug the full NFA
	m_MainNFA.debug();

	// Create DFA lookup table for lexer
	m_MainNFA.convertToDFA(m_LexerTable);

	return m_StatusOk;

} // End of bootstrapLexer()



bool Generator::addNFA(BuildNFA &nfa, const char *tokenName, bool ignoreToken)
{
	// End Of File is hardcoded with id 0 (the first in the list)
	if (m_TokenClasses.size() <= 0)
		m_TokenClasses.emplace_back("END_OF_FILE", false);

	for (size_t n = 0; n < m_TokenClasses.size(); ++n) {
		if (m_TokenClasses[n].name == tokenName) {
			printf("Error: New token %d with name \"%s\" already exist with id %d\n", m_TokenClasses.size(), tokenName, int(n));
			m_StatusOk = false;
			return false;
		}
	}

	int tokenClassId = int(m_TokenClasses.size());
	m_TokenClasses.emplace_back(tokenName, ignoreToken);
	m_MainNFA.addToken(nfa, tokenClassId);

	return true;
}



bool Generator::bootstrapParser()
{
	// Rule <> that calls <Start>
	m_ParserDef.addProduction("<>").addVariable("<Start>").addTerminal("END_OF_FILE");

	// Temp rule <Start>
	m_ParserDef.addProduction("<Start>").addVariable("<Stmts>");
	setVariableClassToSkip("<Start>");

	// Temp rule <Stmts>
	m_ParserDef.addProduction("<Stmts>").addVariable("<TokenDecl>").addVariable("<Stmts>");
	m_ParserDef.addProduction("<Stmts>").addVariable("<RuleDecl>").addVariable("<Stmts>");
	m_ParserDef.addProduction("<Stmts>");
	setVariableClassToSkip("<Stmts>");

	// Rule <TokenDecl>
	m_ParserDef.addProduction("<TokenDecl>").addVariable("<TokenType>").addTerminal("LABEL").addTerminal("EQUAL").addVariable("<TokenExpr>").addTerminal("SEMICOLON");

	// Temp rule <TokenType>
	m_ParserDef.addProduction("<TokenType>").addTerminal("_Token");
	m_ParserDef.addProduction("<TokenType>").addTerminal("_Temp");
	m_ParserDef.addProduction("<TokenType>").addTerminal("_Ignore");
	setVariableClassToSkip("<TokenType>");

	// Temp rule <TokenExpr>
	m_ParserDef.addProduction("<TokenExpr>").addVariable("<TokenExprPar>").addVariable("<TokenExprTail>");
	m_ParserDef.addProduction("<TokenExpr>").addTerminal("LABEL").addVariable("<TokenExprTail>");
	m_ParserDef.addProduction("<TokenExpr>").addTerminal("CONST_CH").addVariable("<TokenExprTail>");
	m_ParserDef.addProduction("<TokenExpr>").addTerminal("CONST_STRING").addVariable("<TokenExprTail>");
	m_ParserDef.addProduction("<TokenExpr>").addTerminal("CONST_SET").addVariable("<TokenExprTail>");
	setVariableClassToSkip("<TokenExpr>");

	// Rule <TokenExprPar>
	m_ParserDef.addProduction("<TokenExprPar>").addTerminal("OPEN_PAR").addVariable("<TokenExpr>").addTerminal("CLOSE_PAR");

	// Temp rule <TokenExprTail>
	m_ParserDef.addProduction("<TokenExprTail>").addVariable("<TokenRepeat>").addVariable("<TokenExprTail>");
	m_ParserDef.addProduction("<TokenExprTail>").addVariable("<TokenExpr>");
	m_ParserDef.addProduction("<TokenExprTail>").addTerminal("OR").addVariable("<TokenExpr>");
	m_ParserDef.addProduction("<TokenExprTail>");
	setVariableClassToSkip("<TokenExprTail>");

	// Rule <TokenRepeat>
	m_ParserDef.addProduction("<TokenRepeat>").addTerminal("ASTERIX");
	m_ParserDef.addProduction("<TokenRepeat>").addTerminal("PLUS");
	m_ParserDef.addProduction("<TokenRepeat>").addTerminal("QUESTION");
	setVariableClassToSkip("<TokenRepeat>");

	// Rule <RuleDecl>
	m_ParserDef.addProduction("<RuleDecl>").addVariable("<RuleType>").addTerminal("VARIABLE").addTerminal("EQUAL").addVariable("<RuleExpr>").addTerminal("SEMICOLON");

	// Temp rule <RuleType>
	m_ParserDef.addProduction("<RuleType>").addTerminal("_Rule");
	m_ParserDef.addProduction("<RuleType>").addTerminal("_Skip");
	setVariableClassToSkip("<RuleType>");

	// Temp rule <RuleExpr>
	m_ParserDef.addProduction("<RuleExpr>").addVariable("<RuleProd>").addVariable("<RuleExprTail>");
	setVariableClassToSkip("<RuleExpr>");

	// Temp rule <RuleExprTail>
	m_ParserDef.addProduction("<RuleExprTail>").addTerminal("OR").addVariable("<RuleExpr>");
	m_ParserDef.addProduction("<RuleExprTail>");
	setVariableClassToSkip("<RuleExprTail>");

	// Rule <RuleProd>
	m_ParserDef.addProduction("<RuleProd>").addTerminal("LABEL").addVariable("<RuleProdTail>");
	m_ParserDef.addProduction("<RuleProd>").addTerminal("VARIABLE").addVariable("<RuleProdTail>");
	m_ParserDef.addProduction("<RuleProd>");

	// Temp rule <RuleProdTail>
	m_ParserDef.addProduction("<RuleProdTail>").addTerminal("LABEL").addVariable("<RuleProdTail>");
	m_ParserDef.addProduction("<RuleProdTail>").addTerminal("VARIABLE").addVariable("<RuleProdTail>");
	m_ParserDef.addProduction("<RuleProdTail>");
	setVariableClassToSkip("<RuleProdTail>");

	bool ok = m_ParserDef.isOk() && m_ParserDef.buildParceTable(m_ParsingTable);

	return ok;
}



bool Generator::setVariableClassToSkip(const char *variableName)
{
	for (size_t n = 0; n < m_VariableClasses.size(); ++n) {
		if (m_VariableClasses[n].name == variableName) {
			m_VariableClasses[n].skip = true;
			return true;
		}
	}

	printf("Error: Set variable \"%s\" as skip failed, missing.\n", variableName);
	m_StatusOk = false;

	return false;
}



bool Generator::saveTables(char *rootName)
{
	bool ok = false;
	FILE *hFp = nullptr, *cppFp = nullptr;
	int id = 0;

	std::string nsName = "Parser_"; nsName += rootName;
	std::string hFilename = nsName + ".h";
	std::string cppFilename = nsName + ".cpp";

	hFp = fopen(hFilename.c_str(), "wt");
	if (!hFp) goto Finished;

	if (fprintf(hFp, "#pragma once\n") < 0) goto Finished;
	if (fprintf(hFp, "#include \"Parser.h\"\n") < 0) goto Finished;
	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "namespace %s {\n", nsName.c_str()) < 0) goto Finished;
	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "	extern TokenClasses g_TokenClasses;\n") < 0) goto Finished;
	if (fprintf(hFp, "	extern VariableClasses g_VariableClasses;\n") < 0) goto Finished;
	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "	extern LexerTable g_LexerTable;\n") < 0) goto Finished;
	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "	extern Productions g_Productions;\n") < 0) goto Finished;
	if (fprintf(hFp, "	extern ParsingTable g_ParsingTable;\n") < 0) goto Finished;

	id = 0;
	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "    enum class TokenId {\n") < 0) goto Finished;
	for (auto it = m_TokenClasses.begin(); it != m_TokenClasses.end(); ++it, ++id) {
		if (fprintf(hFp, "        %s = %d%s\n", it->name.c_str(), id, std::next(it, 1) != m_TokenClasses.end() ? "," : "") < 0) goto Finished;
	}
	if (fprintf(hFp, "    }; // End of enum class TokenId\n") < 0) goto Finished;

	id = 0;
	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "    enum class VariableId {\n") < 0) goto Finished;
	for (auto it = m_VariableClasses.begin(); it != m_VariableClasses.end(); ++it, ++id) {
		std::string str(it->name); str.front() = '_'; str.back() = '_';
		if (fprintf(hFp, "        %s = %d%s\n", str.c_str(), id, std::next(it, 1) != m_VariableClasses.end() ? "," : "") < 0) goto Finished;
	}
	if (fprintf(hFp, "    }; // End of enum class VariableId\n") < 0) goto Finished;

	id = -1;
	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "    enum class SymbolId {\n") < 0) goto Finished;
	for (auto it = m_VariableClasses.begin(); it != m_VariableClasses.end(); ++it, --id) {
		std::string str(it->name); str.front() = '_'; str.back() = '_';
		if (fprintf(hFp, "        %s = %d%s\n", str.c_str(), id, std::next(it, 1) != m_VariableClasses.end() ? "," : "") < 0) goto Finished;
	}
	if (fprintf(hFp, "    }; // End of enum class VariableId\n") < 0) goto Finished;

	if (fprintf(hFp, "\n") < 0) goto Finished;
	if (fprintf(hFp, "} // End of namespace %s\n", nsName.c_str()) < 0) goto Finished;


	cppFp = fopen(cppFilename.c_str(), "wt");
	if (!cppFp) goto Finished;

	if (fprintf(cppFp, "#include \"%s\"\n", hFilename.c_str()) < 0) goto Finished;
	if (fprintf(cppFp, "\n") < 0) goto Finished;
	if (fprintf(cppFp, "namespace %s {\n", nsName.c_str()) < 0) goto Finished;

	id = 0;
	if (fprintf(cppFp, "\n") < 0) goto Finished;
	if (fprintf(cppFp, "    TokenClasses g_TokenClasses = {\n") < 0) goto Finished;
	for (auto it = m_TokenClasses.begin(); it != m_TokenClasses.end(); ++it, ++id) {
		if (fprintf(cppFp, "        {\"%s\", %s}%s//%d\n", it->name.c_str(), it->ignore ? "true" : "false", std::next(it,1) != m_TokenClasses.end() ? "," : "", id) < 0) goto Finished;
	}
	if (fprintf(cppFp, "    }; // End of g_TokenClasses\n") < 0) goto Finished;

	id = 0;
	if (fprintf(cppFp, "\n") < 0) goto Finished;
	if (fprintf(cppFp, "    VariableClasses g_VariableClasses = {\n") < 0) goto Finished;
	for (auto it = m_VariableClasses.begin(); it != m_VariableClasses.end(); ++it, ++id) {
		if (fprintf(cppFp, "        {\"%s\", %s}%s//%d\n", it->name.c_str(), it->skip ? "true" : "false", std::next(it, 1) != m_VariableClasses.end() ? "," : "", id) < 0) goto Finished;
	}
	if (fprintf(cppFp, "    }; // End of g_VariableClasses\n") < 0) goto Finished;

	id = 0;
	if (fprintf(cppFp, "\n") < 0) goto Finished;
	if (fprintf(cppFp, "    LexerTable g_LexerTable = {\n") < 0) goto Finished;
	for (auto it = m_LexerTable.begin(); it != m_LexerTable.end(); ++it, ++id) {
		if (fprintf(cppFp, "        {%3d, //%d  %s\n", it->tokenClassId, id, it->tokenClassId < 0 ? "" : m_TokenClasses[it->tokenClassId].name.c_str()) < 0) goto Finished;
		for (int i = 0; i < 256; ) {
			if (fprintf(cppFp, "            ") < 0) goto Finished;
			for (int j = 0; j < 32; ++j, ++i)
				if (fprintf(cppFp, "%3d%s", it->onChMove[i], i < 255 ? "," : "") < 0) goto Finished;
			if (fprintf(cppFp, "\n") < 0) goto Finished;
		}
		if (fprintf(cppFp, "        }%s\n", std::next(it, 1) != m_LexerTable.end() ? "," : "") < 0) goto Finished;
	}
	if (fprintf(cppFp, "    }; // End of g_LexerTable\n") < 0) goto Finished;

	id = 0;
	if (fprintf(cppFp, "\n") < 0) goto Finished;
	if (fprintf(cppFp, "    Productions g_Productions = {\n") < 0) goto Finished;
	for (auto it = m_Productions.begin(); it != m_Productions.end(); ++it, ++id) {
		if (fprintf(cppFp, "        {%d, {", it->variableClassId) < 0) goto Finished;
		for ( size_t i = 0; i < it->symbols.size(); ++i)
			if (fprintf(cppFp, "%d%s", it->symbols[i], i+1 < it->symbols.size() ? "," : "") < 0) goto Finished;
		if (fprintf(cppFp, "} }%s //%2d  %s ==>", std::next(it, 1) != m_Productions.end() ? "," : "", id, m_VariableClasses[it->variableClassId].name.c_str()) < 0) goto Finished;
		for (size_t i = 0; i < it->symbols.size(); ++i)
			if (fprintf(cppFp, "  %s", it->symbols[i] < 0 ? m_VariableClasses[-(1+it->symbols[i])].name.c_str() : m_TokenClasses[it->symbols[i]].name.c_str()) < 0) goto Finished;
		if (fprintf(cppFp, "\n") < 0) goto Finished;
	}
	if (fprintf(cppFp, "    }; // End of g_Productions\n") < 0) goto Finished;

	id = 0;
	if (fprintf(cppFp, "\n") < 0) goto Finished;
	if (fprintf(cppFp, "    ParsingTable g_ParsingTable = {\n") < 0) goto Finished;
	if (fprintf(cppFp, "      //") < 0) goto Finished;
	for (auto it = m_TokenClasses.begin(); it != m_TokenClasses.end(); ++it) {
		char temp[8];
		strncpy(temp, it->name.c_str(), sizeof(temp));
		temp[5] = 0;
		if (fprintf(cppFp, "%5s,", temp) < 0) goto Finished;
	}
	if (fprintf(cppFp, "\n") < 0) goto Finished;
	for (int variableClassId = 0; variableClassId < int(m_VariableClasses.size()); ++variableClassId) {
		if (fprintf(cppFp, "        ") < 0) goto Finished;
		for (int tokenClassId = 0; tokenClassId < int(m_TokenClasses.size()); ++tokenClassId, ++id) {
			if (fprintf(cppFp, "%5d%s", m_ParsingTable[id], id+1 < int(m_ParsingTable.size()) ? "," : " ") < 0) goto Finished;
		}
		if (fprintf(cppFp, " //%d  %s\n", variableClassId, m_VariableClasses[variableClassId].name.c_str()) < 0) goto Finished;
	}
	if (fprintf(cppFp, "    }; // End of g_ParsingTable\n") < 0) goto Finished;

	if (fprintf(cppFp, "\n") < 0) goto Finished;
	if (fprintf(cppFp, "}\n") < 0) goto Finished;

	ok = true;

Finished:

	if (hFp) fclose(hFp);
	if (cppFp) fclose(cppFp);

	return ok;

}



bool Generator::readCfgFile(char *rootName, RawText &rawText)
{
	bool ok = false;
	FILE *fp = nullptr;

	std::string cfgFilename = rootName; cfgFilename += ".txt";

	fp = fopen(cfgFilename.c_str(), "rb");
	if (!fp) goto Finished;

	if (fseek(fp, 0, SEEK_END) != 0) goto Finished;
	long pos = ftell(fp);
	if (pos < 0) goto Finished;
	rewind(fp);

	size_t lenText = size_t(pos);
	rawText.resize(lenText);
	if (fread(&rawText[0], lenText, 1, fp) < 1) goto Finished;

	ok = true;

Finished:
	if (fp) fclose(fp);
	return ok;
}
