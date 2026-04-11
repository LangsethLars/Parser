#pragma once

#include "ParserBase.h"
#include "Bootstrap_Lexer.h"

class Bootstrap_Parser : public ::ParserBase {

public:

    Bootstrap_Parser() : ::ParserBase(
        g_VariableClasses,
        g_Productions,
        g_ParsingTable,
        Bootstrap_Lexer::g_TokenClasses,
        m_Bootstrap_Lexer.m_TokenSequence,
        m_Bootstrap_Lexer.m_RawText
    ) {}

    bool lexAndParseFile(const char* filename);

    Bootstrap_Lexer m_Bootstrap_Lexer;

    static const VariableClasses g_VariableClasses;
    static const Productions g_Productions;
    static const ParsingTable g_ParsingTable;

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

}; // End of class Bootstrap_Parser
