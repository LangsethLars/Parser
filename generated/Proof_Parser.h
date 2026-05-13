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
        _ParamList_ = 4,
        _PropExpr_ = 5,
        _ProofBlock_ = 6,
        _Param_ = 7,
        _TypeExpr_ = 8,
        _BoolExpr_ = 9,
        _ProofStepList_ = 10,
        _ProofStep_ = 11,
        _AssumeStep_ = 12,
        _HaveStep_ = 13,
        _ExactStep_ = 14,
        _ShowStep_ = 15,
        _CasesStep_ = 16,
        _ReflStep_ = 17,
        _TrivialStep_ = 18,
        _ProofExpr_ = 19,
        _BoolCaseList_ = 20,
        _BoolCase_ = 21,
        _BoolValue_ = 22
    }; // End of enum class VariableId

}; // End of class Proof_Parser
