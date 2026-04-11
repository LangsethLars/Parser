#include "Bootstrap_Parser.h"



bool Bootstrap_Parser::lexAndParseFile(const char* filename) {
    if (!m_Bootstrap_Lexer.lexFile(filename)) {
        printf("Bootstrap_Parser::lexAndParseFile failed to lexFile(\"%s\")\n", filename);
        return false;
    }
    return parseTokenSequence();
}



const VariableClasses Bootstrap_Parser::g_VariableClasses = {
    {"<>", false}, // [0]
    {"<Start>", true}, // [1]
    {"<Stmts>", true}, // [2]
    {"<TokenDecl>", false}, // [3]
    {"<RuleDecl>", false}, // [4]
    {"<TokenType>", true}, // [5]
    {"<TokenExpr>", true}, // [6]
    {"<TokenExprPar>", false}, // [7]
    {"<TokenExprTail>", true}, // [8]
    {"<TokenRepeat>", true}, // [9]
    {"<RuleType>", true}, // [10]
    {"<RuleExpr>", true}, // [11]
    {"<RuleProd>", false}, // [12]
    {"<RuleExprTail>", true}, // [13]
    {"<RuleProdTail>", true} // [14]
}; // End of g_VariableClasses



const Productions Bootstrap_Parser::g_Productions = {
    { 0, { -2, 0 } }, // [0]  <>  ==>  <Start>  END_OF_FILE
    { 1, { -3 } }, // [1]  <Start>  ==>  <Stmts>
    { 2, { -4, -3 } }, // [2]  <Stmts>  ==>  <TokenDecl>  <Stmts>
    { 2, { -5, -3 } }, // [3]  <Stmts>  ==>  <RuleDecl>  <Stmts>
    { 2, {  } }, // [4]  <Stmts>  ==>
    { 3, { -6, 7, 14, -7, 21 } }, // [5]  <TokenDecl>  ==>  <TokenType>  LABEL  EQUAL  <TokenExpr>  SEMICOLON
    { 5, { 9 } }, // [6]  <TokenType>  ==>  _Token
    { 5, { 10 } }, // [7]  <TokenType>  ==>  _Temp
    { 5, { 11 } }, // [8]  <TokenType>  ==>  _Ignore
    { 6, { -8, -9 } }, // [9]  <TokenExpr>  ==>  <TokenExprPar>  <TokenExprTail>
    { 6, { 7, -9 } }, // [10]  <TokenExpr>  ==>  LABEL  <TokenExprTail>
    { 6, { 4, -9 } }, // [11]  <TokenExpr>  ==>  CONST_CH  <TokenExprTail>
    { 6, { 5, -9 } }, // [12]  <TokenExpr>  ==>  CONST_STRING  <TokenExprTail>
    { 6, { 6, -9 } }, // [13]  <TokenExpr>  ==>  CONST_SET  <TokenExprTail>
    { 7, { 15, -7, 16 } }, // [14]  <TokenExprPar>  ==>  OPEN_PAR  <TokenExpr>  CLOSE_PAR
    { 8, { -10, -9 } }, // [15]  <TokenExprTail>  ==>  <TokenRepeat>  <TokenExprTail>
    { 8, { -7 } }, // [16]  <TokenExprTail>  ==>  <TokenExpr>
    { 8, { 20, -7 } }, // [17]  <TokenExprTail>  ==>  OR  <TokenExpr>
    { 8, {  } }, // [18]  <TokenExprTail>  ==>
    { 9, { 17 } }, // [19]  <TokenRepeat>  ==>  ASTERIX
    { 9, { 18 } }, // [20]  <TokenRepeat>  ==>  PLUS
    { 9, { 19 } }, // [21]  <TokenRepeat>  ==>  QUESTION
    { 4, { -11, 8, 14, -12, 21 } }, // [22]  <RuleDecl>  ==>  <RuleType>  VARIABLE  EQUAL  <RuleExpr>  SEMICOLON
    { 10, { 12 } }, // [23]  <RuleType>  ==>  _Rule
    { 10, { 13 } }, // [24]  <RuleType>  ==>  _Skip
    { 11, { -13, -14 } }, // [25]  <RuleExpr>  ==>  <RuleProd>  <RuleExprTail>
    { 13, { 20, -12 } }, // [26]  <RuleExprTail>  ==>  OR  <RuleExpr>
    { 13, {  } }, // [27]  <RuleExprTail>  ==>
    { 12, { 7, -15 } }, // [28]  <RuleProd>  ==>  LABEL  <RuleProdTail>
    { 12, { 8, -15 } }, // [29]  <RuleProd>  ==>  VARIABLE  <RuleProdTail>
    { 12, {  } }, // [30]  <RuleProd>  ==>
    { 14, { 7, -15 } }, // [31]  <RuleProdTail>  ==>  LABEL  <RuleProdTail>
    { 14, { 8, -15 } }, // [32]  <RuleProdTail>  ==>  VARIABLE  <RuleProdTail>
    { 14, {  } } // [33]  <RuleProdTail>  ==>
}; // End of g_Productions



const ParsingTable Bootstrap_Parser::g_ParsingTable = {
//END_O,COMME,SPACE,   NL,CONST,CONST,CONST,LABEL,VARIA,_Toke,_Temp,_Igno,_Rule,_Skip,EQUAL,OPEN_,CLOSE,ASTER, PLUS,QUEST,   OR,SEMIC
      0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    0,    0,    0,    0,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [0] <>
      1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    1,    1,    1,    1,    1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [1] <Start>
      4,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    2,    2,    2,    3,    3,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [2] <Stmts>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    5,    5,    5,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [3] <TokenDecl>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   22,   22,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [4] <RuleDecl>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    6,    7,    8,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [5] <TokenType>
     -1,   -1,   -1,   -1,   11,   12,   13,   10,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    9,   -1,   -1,   -1,   -1,   -1,   -1, // [6] <TokenExpr>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   14,   -1,   -1,   -1,   -1,   -1,   -1, // [7] <TokenExprPar>
     -1,   -1,   -1,   -1,   16,   16,   16,   16,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   16,   18,   15,   15,   15,   17,   18, // [8] <TokenExprTail>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   19,   20,   21,   -1,   -1, // [9] <TokenRepeat>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   23,   24,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [10] <RuleType>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   25,   25,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   25,   25, // [11] <RuleExpr>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   28,   29,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   30,   30, // [12] <RuleProd>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   26,   27, // [13] <RuleExprTail>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   31,   32,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   33,   33  // [14] <RuleProdTail>
}; // End of g_ParsingTable
