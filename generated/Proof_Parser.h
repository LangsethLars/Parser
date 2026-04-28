#pragma once

#include "ParserBase.h"
#include "Proof_Lexer.h"

class Proof_Parser : public ::ParserBase {

public:

    Proof_Parser() : ::ParserBase(
        g_VariableClasses,
        g_Productions,
        g_ParsingTable,
        Proof_Lexer::g_TokenClasses,
        m_Proof_Lexer.m_TokenSequence,
        m_Proof_Lexer.m_RawText
    ) {}

    bool lexAndParseFile(const char* filename);

    Proof_Lexer m_Proof_Lexer;

    static const VariableClasses g_VariableClasses;
    static const Productions g_Productions;
    static const ParsingTable g_ParsingTable;

    enum class VariableId {
        __ = 0,
        _Start_ = 1,
        _Decls_ = 2,
        _Decl_ = 3,
        _DeclsLoop_ = 4,
        _Expr_ = 5,
        _FunExpr_ = 6,
        _PiExpr_ = 7,
        _AppExpr_ = 8,
        _Atom_ = 9,
        _AppLoop_ = 10,
        _AppListTail_ = 11
    }; // End of enum class VariableId

}; // End of class Proof_Parser
