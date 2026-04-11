#include "BuildBootstrap.h"



bool BuildBootstrap::bootstrapLexerAndParser()
{
	m_MainNFA.clear();
	return bootstrapLexer() && bootstrapParser();
}



bool BuildBootstrap::bootstrapLexer()
{
	BuildNFA nfa, nfa2;
	bool ignore = true;
	bool ok = true;

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
	nfa.clear().appendCh('#');
	nfa2.clear().appendChSet(chSet_All).timesZeroOrMore();
	nfa.appendNFA(nfa2);
	ok &= m_MainNFA.addToken(nfa, "COMMENT", ignore);

	// Ignore SPACE = sp+;
	nfa.clear().appendChSet(chSet_Sp).timesOneOrMore();
	ok &= m_MainNFA.addToken(nfa, "SPACE", ignore);

	// Ignore NL = '\0A' | '\0D' | "\0D\0A" | "\0A\0D";  # PS: Maximum length match, will cover most cases.
	nfa.clear().appendCh(10);
	nfa2.clear().appendCh(13);
	nfa.this_or(nfa2);
	nfa2.clear().appendCh(13).appendCh(10);
	nfa.this_or(nfa2);
	nfa2.clear().appendCh(10).appendCh(13);
	nfa.this_or(nfa2);
	ok &= m_MainNFA.addToken(nfa, "NL", ignore);

	// ChSet h = [0-9A-Fa-f]; # Hexadecimal digits.
	ChSet chSet_Hex;
	chSet_Hex.addRange('0', '9');
	chSet_Hex.addRange('A', 'F');
	chSet_Hex.addRange('a', 'f');

	// ChSet g = [!"#$%&'()*+,\-./:;<=>?@[\\\]^_`{|}\~]; # Visible graphical characters (not alphabetic or digit).
	ChSet chSet_Graphical;
	chSet_Graphical.addChars((const uint8_t*)("!\"#$%&'()*+,-./:;<=>?@[\\]^_'{|}~"));

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
	nfa.clear().appendCh('\'').appendNFA(nfa2).appendCh('\'');
	ok &= m_MainNFA.addToken(nfa, "CONST_CH");

	// Token CONST_STRING = '"' ([\01-\FF~\0A\0D"\\] | ESC_CH)* '"';
	ChSet chSet_ConstStringCh;
	chSet_ConstStringCh.addRange(1, 255);
	chSet_ConstStringCh.removeChar(10);
	chSet_ConstStringCh.removeChar(13);
	chSet_ConstStringCh.removeChar('"');
	chSet_ConstStringCh.removeChar('\\');
	nfa2.clear().appendChSet(chSet_ConstStringCh).this_or(nfa_ESC_CH).timesZeroOrMore();
	nfa.clear().appendCh('"').appendNFA(nfa2).appendCh('"');
	ok &= m_MainNFA.addToken(nfa, "CONST_STRING");

	// Token CONST_SET = '[' CONST_SET_CHARS ('~' CONST_SET_CHARS)? ']';
	nfa2.clear().appendCh('~').appendNFA(nfa_CONST_SET_CHARS).timesZeroOrOne();
	nfa.clear().appendCh('[').appendNFA(nfa_CONST_SET_CHARS).appendNFA(nfa2).appendCh(']');
	ok &= m_MainNFA.addToken(nfa, "CONST_SET");

	ChSet chSet_Alpha_;
	chSet_Alpha_.addRange('A', 'Z');
	chSet_Alpha_.addRange('a', 'z');
	chSet_Alpha_.addChar('_');

	ChSet chSet_AlphaNum_;
	chSet_AlphaNum_.addRange('A', 'Z');
	chSet_AlphaNum_.addRange('a', 'z');
	chSet_AlphaNum_.addRange('0', '9');
	chSet_AlphaNum_.addChar('_');

	// Temp  ALPHANUM  = [_A-Za-z] [_A-Za-z0-9]*;
	BuildNFA nfa_ALPHANUM;
	nfa2.clear().appendChSet(chSet_AlphaNum_).timesZeroOrMore();
	nfa_ALPHANUM.clear().appendChSet(chSet_Alpha_).appendNFA(nfa2);

	// Token LABEL = [_A-Za-z] [_A-Za-z0-9]*;
	// nfa2.clear().appendChSet(chSet_AlphaNum_).timesZeroOrMore();
	// nfa.clear().appendChSet(chSet_Alpha_).appendNFA(nfa2);
	nfa.clear().appendNFA(nfa_ALPHANUM);
	ok &= m_MainNFA.addToken(nfa, "LABEL");

	// Token VARIABLE = '<' [_A-Za-z] [_A-Za-z0-9]* '>';
	// nfa2.clear().appendChSet(chSet_AlphaNum_).timesZeroOrMore();
	// nfa.clear().appendCh('<').appendChSet(chSet_Alpha_).appendNFA(nfa2).appendCh('>');
	nfa.clear().appendCh('<').appendNFA(nfa_ALPHANUM).appendCh('>');
	ok &= m_MainNFA.addToken(nfa, "VARIABLE");

	// Token _Token = "Token";
	nfa.clear().appendStr("Token");
	ok &= m_MainNFA.addToken(nfa, "_Token");

	// Token _Temp = "Temp";
	nfa.clear().appendStr("Temp");
	ok &= m_MainNFA.addToken(nfa, "_Temp");

	// Token _Ignore = "Ignore";
	nfa.clear().appendStr("Ignore");
	ok &= m_MainNFA.addToken(nfa, "_Ignore");

	// Token _Rule_ = "Rule";
	nfa.clear().appendStr("Rule");
	ok &= m_MainNFA.addToken(nfa, "_Rule");

	// Token _Skip_ = "Skip";
	nfa.clear().appendStr("Skip");
	ok &= m_MainNFA.addToken(nfa, "_Skip");

	// Token EQUAL = '=';
	nfa.clear().appendCh('=');
	ok &= m_MainNFA.addToken(nfa, "EQUAL");

	// Token OPEN_PAR = '(';
	nfa.clear().appendCh('(');
	ok &= m_MainNFA.addToken(nfa, "OPEN_PAR");

	// Token CLOSE_PAR = ')';
	nfa.clear().appendCh(')');
	ok &= m_MainNFA.addToken(nfa, "CLOSE_PAR");

	// Token ASTERIX = '*';
	nfa.clear().appendCh('*');
	ok &= m_MainNFA.addToken(nfa, "ASTERIX");

	// Token PLUS = '+';
	nfa.clear().appendCh('+');
	ok &= m_MainNFA.addToken(nfa, "PLUS");

	// Token QUESTION = '?';
	nfa.clear().appendCh('?');
	ok &= m_MainNFA.addToken(nfa, "QUESTION");

	// Token OR = '|';
	nfa.clear().appendCh('|');
	ok &= m_MainNFA.addToken(nfa, "OR");

	// Token SEMICOLON = ';';
	nfa.clear().appendCh(';');
	ok &= m_MainNFA.addToken(nfa, "SEMICOLON");

	// Debug the full NFA
	m_MainNFA.debugNFA();

	// Create DFA lookup table for lexer
	m_MainNFA.convertToDFA();
	ok &= m_MainNFA.saveDFA("Bootstrap");

	return ok;

} // End of bootstrapLexer()



bool BuildBootstrap::bootstrapParser()
{
	bool ok = true;

	// Rule <> that calls <Start>
	m_MainCFG.addProduction("<>").addVariable("<Start>").addTerminal("END_OF_FILE");

	// Temp rule <Start>
	m_MainCFG.addProduction("<Start>").addVariable("<Stmts>");
	m_MainCFG.setVariableClassToSkip("<Start>");

	// Temp rule <Stmts>
	m_MainCFG.addProduction("<Stmts>").addVariable("<TokenDecl>").addVariable("<Stmts>");
	m_MainCFG.addProduction("<Stmts>").addVariable("<RuleDecl>").addVariable("<Stmts>");
	m_MainCFG.addProduction("<Stmts>");
	m_MainCFG.setVariableClassToSkip("<Stmts>");

	// Rule <TokenDecl>
	m_MainCFG.addProduction("<TokenDecl>").addVariable("<TokenType>").addTerminal("LABEL").addTerminal("EQUAL").addVariable("<TokenExpr>").addTerminal("SEMICOLON");

	// Temp rule <TokenType>
	m_MainCFG.addProduction("<TokenType>").addTerminal("_Token");
	m_MainCFG.addProduction("<TokenType>").addTerminal("_Temp");
	m_MainCFG.addProduction("<TokenType>").addTerminal("_Ignore");
	m_MainCFG.setVariableClassToSkip("<TokenType>");

	// Temp rule <TokenExpr>
	m_MainCFG.addProduction("<TokenExpr>").addVariable("<TokenExprPar>").addVariable("<TokenExprTail>");
	m_MainCFG.addProduction("<TokenExpr>").addTerminal("LABEL").addVariable("<TokenExprTail>");
	m_MainCFG.addProduction("<TokenExpr>").addTerminal("CONST_CH").addVariable("<TokenExprTail>");
	m_MainCFG.addProduction("<TokenExpr>").addTerminal("CONST_STRING").addVariable("<TokenExprTail>");
	m_MainCFG.addProduction("<TokenExpr>").addTerminal("CONST_SET").addVariable("<TokenExprTail>");
	m_MainCFG.setVariableClassToSkip("<TokenExpr>");

	// Rule <TokenExprPar>
	m_MainCFG.addProduction("<TokenExprPar>").addTerminal("OPEN_PAR").addVariable("<TokenExpr>").addTerminal("CLOSE_PAR");
	// Temp rule <TokenExprTail>
	m_MainCFG.addProduction("<TokenExprTail>").addVariable("<TokenRepeat>").addVariable("<TokenExprTail>");
	m_MainCFG.addProduction("<TokenExprTail>").addVariable("<TokenExpr>");
	m_MainCFG.addProduction("<TokenExprTail>").addTerminal("OR").addVariable("<TokenExpr>");
	m_MainCFG.addProduction("<TokenExprTail>");
	m_MainCFG.setVariableClassToSkip("<TokenExprTail>");

	// Rule <TokenRepeat>
	m_MainCFG.addProduction("<TokenRepeat>").addTerminal("ASTERIX");
	m_MainCFG.addProduction("<TokenRepeat>").addTerminal("PLUS");
	m_MainCFG.addProduction("<TokenRepeat>").addTerminal("QUESTION");
	m_MainCFG.setVariableClassToSkip("<TokenRepeat>");

	// Rule <RuleDecl>
	m_MainCFG.addProduction("<RuleDecl>").addVariable("<RuleType>").addTerminal("VARIABLE").addTerminal("EQUAL").addVariable("<RuleExpr>").addTerminal("SEMICOLON");

	// Temp rule <RuleType>
	m_MainCFG.addProduction("<RuleType>").addTerminal("_Rule");
	m_MainCFG.addProduction("<RuleType>").addTerminal("_Skip");
	m_MainCFG.setVariableClassToSkip("<RuleType>");

	// Temp rule <RuleExpr>
	m_MainCFG.addProduction("<RuleExpr>").addVariable("<RuleProd>").addVariable("<RuleExprTail>");
	m_MainCFG.setVariableClassToSkip("<RuleExpr>");

	// Temp rule <RuleExprTail>
	m_MainCFG.addProduction("<RuleExprTail>").addTerminal("OR").addVariable("<RuleExpr>");
	m_MainCFG.addProduction("<RuleExprTail>");
	m_MainCFG.setVariableClassToSkip("<RuleExprTail>");

	// Rule <RuleProd>
	m_MainCFG.addProduction("<RuleProd>").addTerminal("LABEL").addVariable("<RuleProdTail>");
	m_MainCFG.addProduction("<RuleProd>").addTerminal("VARIABLE").addVariable("<RuleProdTail>");
	m_MainCFG.addProduction("<RuleProd>");

	// Temp rule <RuleProdTail>
	m_MainCFG.addProduction("<RuleProdTail>").addTerminal("LABEL").addVariable("<RuleProdTail>");
	m_MainCFG.addProduction("<RuleProdTail>").addTerminal("VARIABLE").addVariable("<RuleProdTail>");
	m_MainCFG.addProduction("<RuleProdTail>");
	m_MainCFG.setVariableClassToSkip("<RuleProdTail>");

	ok = m_MainCFG.isOk() && m_MainCFG.buildParseTable();
	m_MainCFG.saveCFG("Bootstrap");

	return ok;

} // End of bootstrap()
