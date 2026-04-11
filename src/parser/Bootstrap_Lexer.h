#pragma once

#include "LexerBase.h"

class Bootstrap_Lexer : public ::LexerBase {

public:

    Bootstrap_Lexer() : ::LexerBase(g_TokenClasses, g_LexerTable) {}

    static const TokenClasses g_TokenClasses;
    static const LexerTable g_LexerTable;

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

}; // End of class Bootstrap_Lexer
