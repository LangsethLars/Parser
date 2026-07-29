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
        NUMBER = 5,
        _Prop = 6,
        _Prop_True = 7,
        _Prop_False = 8,
        _Prop_Not = 9,
        _Prop_And = 10,
        _Prop_Or = 11,
        _Prop_Implies = 12,
        _Prop_Iff = 13,
        _Prop_AndLeft = 14,
        _Prop_AndRight = 15,
        _Prop_AndIntro = 16,
        _Prop_OrLeft = 17,
        _Prop_OrRight = 18,
        _Prop_FalseElim = 19,
        _Prop_Eq = 20,
        _Theorem = 21,
        _Proof = 22,
        _End = 23,
        _Assume = 24,
        _Have = 25,
        _Exact = 26,
        _Show = 27,
        _Refl = 28,
        _Trivial = 29,
        _Inductive = 30,
        _Type = 31,
        _Def = 32,
        _Match = 33,
        _Case = 34,
        _By = 35,
        _Rewrite = 36,
        _Symm = 37,
        _Trans = 38,
        _Reduce = 39,
        _ForAll = 40,
        _Exists = 41,
        _Induction = 42,
        _Fun = 43,
        _Pi = 44,
        _Nat = 45,
        EQUAL = 46,
        OPEN_PAR = 47,
        CLOSE_PAR = 48,
        COLON = 49,
        COLONEQUAL = 50,
        COMMA = 51,
        DOT = 52,
        SLIM_ARROW = 53,
        FAT_ARROW = 54,
        SEMICOLON = 55,
        PIPE = 56
    }; // End of enum class TokenId

}; // End of class Proof_Lexer
