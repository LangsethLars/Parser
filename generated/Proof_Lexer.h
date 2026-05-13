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
        _Bool = 5,
        _Bool_True = 6,
        _Bool_False = 7,
        _Bool_Not = 8,
        _Bool_And = 9,
        _Bool_Or = 10,
        _Bool_Eq = 11,
        _Prop = 12,
        _Prop_True = 13,
        _Prop_False = 14,
        _Prop_Not = 15,
        _Prop_And = 16,
        _Prop_Or = 17,
        _Prop_Implies = 18,
        _Prop_Iff = 19,
        _Prop_AndLeft = 20,
        _Prop_AndRight = 21,
        _Prop_AndIntro = 22,
        _Prop_OrLeft = 23,
        _Prop_OrRight = 24,
        _Prop_FalseElim = 25,
        _Theorem = 26,
        _Proof = 27,
        _End = 28,
        _Assume = 29,
        _Have = 30,
        _Exact = 31,
        _Show = 32,
        _Cases = 33,
        _Case = 34,
        _Refl = 35,
        _Trivial = 36,
        _By = 37,
        _Rewrite = 38,
        _Symm = 39,
        _Trans = 40,
        _Reduce = 41,
        _Def = 42,
        _ForAll = 43,
        _Exists = 44,
        _Induction = 45,
        _Fun = 46,
        _Pi = 47,
        _Type = 48,
        _Nat = 49,
        _Int = 50,
        EQUAL = 51,
        OPEN_PAR = 52,
        CLOSE_PAR = 53,
        COLON = 54,
        COLONEQUAL = 55,
        COMMA = 56,
        DOT = 57,
        SLIM_ARROW = 58,
        FAT_ARROW = 59,
        SEMICOLON = 60
    }; // End of enum class TokenId

}; // End of class Proof_Lexer
