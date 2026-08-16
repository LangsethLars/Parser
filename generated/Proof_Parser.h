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
        _Item_ = 2,
        _Inductive_ = 3,
        _Def_ = 4,
        _RegisterEquality_ = 5,
        _Theorem_ = 6,
        _ConstructorList_ = 7,
        _Constructor_ = 8,
        _ParamList_ = 9,
        _TypeExpr_ = 10,
        _Statements_ = 11,
        _ArgExpr_ = 12,
        _Param_ = 13,
        _ArgList_ = 14,
        _Args_ = 15,
        _Statement_ = 16,
        _Cases_ = 17,
        _ICases_ = 18,
        _Case_ = 19,
        _ICase_ = 20
    }; // End of enum class VariableId

}; // End of class Proof_Parser
