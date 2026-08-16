#pragma once

#include "LexerBase.h"

class Proof_Lexer : public ::LexerBase {

public:

    Proof_Lexer() : ::LexerBase(g_TokenClasses, g_LexerTable) {}

    static const TokenClasses g_TokenClasses;
    static const LexerTable g_LexerTable;

    enum class TokenId {
        END_OF_FILE = 0,
        COMMENT = 1,
        SPACE = 2,
        NL = 3,
        IDENT = 4,
        _Prop = 5,
        _Prop_True = 6,
        _Prop_False = 7,
        _RegisterEquality = 8,
        _Def = 9,
        _Let = 10,
        _Match = 11,
        _Case = 12,
        _Else = 13,
        _End = 14,
        _Expression = 15,
        _Theorem = 16,
        _Proof = 17,
        _Eval = 18,
        _Trivial = 19,
        _Induction = 20,
        _Rewrite = 21,
        _Inductive = 22,
        _Type = 23,
        COLONEQUAL = 24,
        EQUAL = 25,
        OPEN_PAR = 26,
        CLOSE_PAR = 27,
        COLON = 28,
        SLIM_ARROW = 29,
        FAT_ARROW = 30,
        PIPE = 31
    }; // End of enum class TokenId

}; // End of class Proof_Lexer
