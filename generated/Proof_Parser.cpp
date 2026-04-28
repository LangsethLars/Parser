#include "Proof_Parser.h"



bool Proof_Parser::lexAndParseFile(const char* filename) {
    if (!m_Proof_Lexer.lexFile(filename)) {
        printf("Proof_Parser::lexAndParseFile failed to lexFile(\"%s\")\n", filename);
        return false;
    }
    return parseTokenSequence();
}



const VariableClasses Proof_Parser::g_VariableClasses = {
    {"<>", false}, // [0]
    {"<Start>", true}, // [1]
    {"<Decls>", true}, // [2]
    {"<Decl>", false}, // [3]
    {"<DeclsLoop>", true}, // [4]
    {"<Expr>", false}, // [5]
    {"<FunExpr>", false}, // [6]
    {"<PiExpr>", false}, // [7]
    {"<AppExpr>", false}, // [8]
    {"<Atom>", true}, // [9]
    {"<AppLoop>", true}, // [10]
    {"<AppListTail>", true} // [11]
}; // End of g_VariableClasses



const Productions Proof_Parser::g_Productions = {
    { 0, { -2, 0 } }, // [0]  <>  ==>  <Start>  END_OF_FILE
    { 1, { -3 } }, // [1]  <Start>  ==>  <Decls>
    { 2, { -4, -5 } }, // [2]  <Decls>  ==>  <Decl>  <DeclsLoop>
    { 2, {  } }, // [3]  <Decls>  ==>
    { 4, { 14, -4, -5 } }, // [4]  <DeclsLoop>  ==>  SEMICOLON  <Decl>  <DeclsLoop>
    { 4, {  } }, // [5]  <DeclsLoop>  ==>
    { 3, { -6 } }, // [6]  <Decl>  ==>  <Expr>
    { 5, { -7 } }, // [7]  <Expr>  ==>  <FunExpr>
    { 5, { -8 } }, // [8]  <Expr>  ==>  <PiExpr>
    { 5, { -9 } }, // [9]  <Expr>  ==>  <AppExpr>
    { 6, { 6, 5, 11, -6, 13, -6 } }, // [10]  <FunExpr>  ==>  _fun  IDENT  COLON  <Expr>  FAT_ARROW  <Expr>
    { 7, { 7, 5, 11, -6, 12, -6 } }, // [11]  <PiExpr>  ==>  _Pi  IDENT  COLON  <Expr>  COMMA  <Expr>
    { 8, { -10, -11 } }, // [12]  <AppExpr>  ==>  <Atom>  <AppLoop>
    { 10, { -12 } }, // [13]  <AppLoop>  ==>  <AppListTail>
    { 10, {  } }, // [14]  <AppLoop>  ==>
    { 11, { -10, -11 } }, // [15]  <AppListTail>  ==>  <Atom>  <AppLoop>
    { 9, { 5 } }, // [16]  <Atom>  ==>  IDENT
    { 9, { 8 } }, // [17]  <Atom>  ==>  _Type
    { 9, { 9, -6, 10 } } // [18]  <Atom>  ==>  OPEN_PAR  <Expr>  CLOSE_PAR
}; // End of g_Productions



const ParsingTable Proof_Parser::g_ParsingTable = {
//END_O,COMME,SPACE,   NL,CONST,IDENT, _fun,  _Pi,_Type,OPEN_,CLOSE,COLON,COMMA,FAT_A,SEMIC
      0,   -1,   -1,   -1,   -1,    0,    0,    0,    0,    0,   -1,   -1,   -1,   -1,   -1, // [0] <>
      1,   -1,   -1,   -1,   -1,    1,    1,    1,    1,    1,   -1,   -1,   -1,   -1,   -1, // [1] <Start>
      3,   -1,   -1,   -1,   -1,    2,    2,    2,    2,    2,   -1,   -1,   -1,   -1,   -1, // [2] <Decls>
     -1,   -1,   -1,   -1,   -1,    6,    6,    6,    6,    6,   -1,   -1,   -1,   -1,   -1, // [3] <Decl>
      5,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    4, // [4] <DeclsLoop>
     -1,   -1,   -1,   -1,   -1,    9,    7,    8,    9,    9,   -1,   -1,   -1,   -1,   -1, // [5] <Expr>
     -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [6] <FunExpr>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   11,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [7] <PiExpr>
     -1,   -1,   -1,   -1,   -1,   12,   -1,   -1,   12,   12,   -1,   -1,   -1,   -1,   -1, // [8] <AppExpr>
     -1,   -1,   -1,   -1,   -1,   16,   -1,   -1,   17,   18,   -1,   -1,   -1,   -1,   -1, // [9] <Atom>
     14,   -1,   -1,   -1,   -1,   13,   -1,   -1,   13,   13,   14,   -1,   14,   14,   14, // [10] <AppLoop>
     -1,   -1,   -1,   -1,   -1,   15,   -1,   -1,   15,   15,   -1,   -1,   -1,   -1,   -1  // [11] <AppListTail>
}; // End of g_ParsingTable
