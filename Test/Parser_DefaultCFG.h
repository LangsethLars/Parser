#pragma once
#include "Parser.h"

namespace Parser_DefaultCFG {

	extern TokenClasses g_TokenClasses;
	extern VariableClasses g_VariableClasses;

	extern LexerTable g_LexerTable;

	extern Productions g_Productions;
	extern ParsingTable g_ParsingTable;

    enum class TokenId {
        END_OF_FILE = 0,
        COMMENT = 1,
        SPACE = 2,
        NL = 3,
        CONST_CH = 4,
        CONST_STRING = 5,
        CONST_SET = 6,
        LABEL = 7,
        VARIABLE = 8,
        _Token = 9,
        _Temp = 10,
        _Ignore = 11,
        _Rule = 12,
        _Skip = 13,
        EQUAL = 14,
        OPEN_PAR = 15,
        CLOSE_PAR = 16,
        ASTERIX = 17,
        PLUS = 18,
        QUESTION = 19,
        OR = 20,
        SEMICOLON = 21
    }; // End of enum class TokenId

    enum class VariableId {
        __ = 0,
        _Start_ = 1,
        _Stmts_ = 2,
        _TokenDecl_ = 3,
        _RuleDecl_ = 4,
        _TokenType_ = 5,
        _TokenExpr_ = 6,
        _TokenExprPar_ = 7,
        _TokenExprTail_ = 8,
        _TokenRepeat_ = 9,
        _RuleType_ = 10,
        _RuleExpr_ = 11,
        _RuleProd_ = 12,
        _RuleExprTail_ = 13,
        _RuleProdTail_ = 14
    }; // End of enum class VariableId

    enum class SymbolId {
        __ = -1,
        _Start_ = -2,
        _Stmts_ = -3,
        _TokenDecl_ = -4,
        _RuleDecl_ = -5,
        _TokenType_ = -6,
        _TokenExpr_ = -7,
        _TokenExprPar_ = -8,
        _TokenExprTail_ = -9,
        _TokenRepeat_ = -10,
        _RuleType_ = -11,
        _RuleExpr_ = -12,
        _RuleProd_ = -13,
        _RuleExprTail_ = -14,
        _RuleProdTail_ = -15
    }; // End of enum class VariableId

} // End of namespace Parser_DefaultCFG
