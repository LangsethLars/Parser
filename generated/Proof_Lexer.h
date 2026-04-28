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
        CONST_STRING = 4,
        IDENT = 5,
        _fun = 6,
        _Pi = 7,
        _Type = 8,
        OPEN_PAR = 9,
        CLOSE_PAR = 10,
        COLON = 11,
        COMMA = 12,
        FAT_ARROW = 13,
        SEMICOLON = 14
    }; // End of enum class TokenId

}; // End of class Proof_Lexer
