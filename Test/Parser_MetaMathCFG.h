#pragma once
#include "Parser.h"

namespace Parser_MetaMathCFG {

	extern TokenClasses g_TokenClasses;
	extern VariableClasses g_VariableClasses;

	extern LexerTable g_LexerTable;

	extern Productions g_Productions;
	extern ParsingTable g_ParsingTable;

    enum class TokenId {
        END_OF_FILE = 0,
        COMMENT = 1,
        WHITESPACE = 2,
        INCLUDE = 3,
        LABEL = 4,
        MATH = 5,
        BEGIN = 6,
        END = 7,
        CONST = 8,
        VAR = 9,
        DISJOINT = 10,
        FLOATING = 11,
        LOGICAL = 12,
        AXIOMATIC = 13,
        THEOREM = 14,
        PROOF = 15,
        STOP = 16,
        ERROR = 17
    }; // End of enum class TokenId

    enum class VariableId {
        __ = 0,
        _Start_ = 1,
        _Stmts_ = 2,
        _Math_ = 3,
        _BlockDecl_ = 4,
        _ConstDecl_ = 5,
        _VarDecl_ = 6,
        _DisjointDecl_ = 7,
        _LabelDecl_ = 8,
        _ConstExprTail_ = 9,
        _VarExprTail_ = 10,
        _DisjointExprTail_ = 11,
        _LabelDeclTail_ = 12,
        _MathExprTail_ = 13
    }; // End of enum class VariableId

    enum class SymbolId {
        __ = -1,
        _Start_ = -2,
        _Stmts_ = -3,
        _Math_ = -4,
        _BlockDecl_ = -5,
        _ConstDecl_ = -6,
        _VarDecl_ = -7,
        _DisjointDecl_ = -8,
        _LabelDecl_ = -9,
        _ConstExprTail_ = -10,
        _VarExprTail_ = -11,
        _DisjointExprTail_ = -12,
        _LabelDeclTail_ = -13,
        _MathExprTail_ = -14
    }; // End of enum class VariableId

} // End of namespace Parser_MetaMathCFG
