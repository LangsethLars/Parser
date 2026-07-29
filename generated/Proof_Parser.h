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
        _Theorem_ = 3,
        _Inductive_ = 4,
        _Def_ = 5,
        _ParamList_ = 6,
        _Param_ = 7,
        _Sort_ = 8,
        _TypeExpr_ = 9,
        _ArgExpr_ = 10,
        _ArgList_ = 11,
        _Args_ = 12,
        _PropExpr_ = 13,
        _ProofBlock_ = 14,
        _ProofStepList_ = 15,
        _ProofStep_ = 16,
        _AssumeStep_ = 17,
        _HaveStep_ = 18,
        _ExactStep_ = 19,
        _ShowStep_ = 20,
        _CasesStep_ = 21,
        _ReflStep_ = 22,
        _TrivialStep_ = 23,
        _ProofExpr_ = 24,
        _ConstructorList_ = 25,
        _Constructor_ = 26,
        _DefExpr_ = 27,
        _MatchExpr_ = 28,
        _CaseList_ = 29
    }; // End of enum class VariableId

}; // End of class Proof_Parser
