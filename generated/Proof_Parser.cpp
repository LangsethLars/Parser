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
    {"<Item>", true}, // [2]
    {"<Inductive>", false}, // [3]
    {"<Def>", false}, // [4]
    {"<RegisterEquality>", false}, // [5]
    {"<Theorem>", false}, // [6]
    {"<ConstructorList>", true}, // [7]
    {"<Constructor>", false}, // [8]
    {"<ParamList>", true}, // [9]
    {"<TypeExpr>", false}, // [10]
    {"<Statements>", true}, // [11]
    {"<ArgExpr>", false}, // [12]
    {"<Param>", false}, // [13]
    {"<ArgList>", true}, // [14]
    {"<Args>", true}, // [15]
    {"<Statement>", false}, // [16]
    {"<Cases>", true}, // [17]
    {"<ICases>", true}, // [18]
    {"<Case>", true}, // [19]
    {"<ICase>", true} // [20]
}; // End of g_VariableClasses



const Productions Proof_Parser::g_Productions = {
    { 0, { -2, 0 } }, // [0]  <>  ==>  <Start>  END_OF_FILE
    { 1, { -3 } }, // [1]  <Start>  ==>  <Item>
    { 2, { -4, -3 } }, // [2]  <Item>  ==>  <Inductive>  <Item>
    { 2, { -5, -3 } }, // [3]  <Item>  ==>  <Def>  <Item>
    { 2, { -6, -3 } }, // [4]  <Item>  ==>  <RegisterEquality>  <Item>
    { 2, { -7, -3 } }, // [5]  <Item>  ==>  <Theorem>  <Item>
    { 2, {  } }, // [6]  <Item>  ==>
    { 3, { 22, 4, 28, 23, -8, 14 } }, // [7]  <Inductive>  ==>  _Inductive  IDENT  COLON  _Type  <ConstructorList>  _End
    { 7, { -9, -8 } }, // [8]  <ConstructorList>  ==>  <Constructor>  <ConstructorList>
    { 7, {  } }, // [9]  <ConstructorList>  ==>
    { 8, { 31, 4, -10 } }, // [10]  <Constructor>  ==>  PIPE  IDENT  <ParamList>
    { 4, { 9, 4, -10, 28, -11, 24, -12, 14 } }, // [11]  <Def>  ==>  _Def  IDENT  <ParamList>  COLON  <TypeExpr>  COLONEQUAL  <Statements>  _End
    { 5, { 8, 4 } }, // [12]  <RegisterEquality>  ==>  _RegisterEquality  IDENT
    { 6, { 16, 4, -10, 28, -13, 17, -12, 14 } }, // [13]  <Theorem>  ==>  _Theorem  IDENT  <ParamList>  COLON  <ArgExpr>  _Proof  <Statements>  _End
    { 9, { -14, -10 } }, // [14]  <ParamList>  ==>  <Param>  <ParamList>
    { 9, {  } }, // [15]  <ParamList>  ==>
    { 13, { 26, 4, 28, -11, 27 } }, // [16]  <Param>  ==>  OPEN_PAR  IDENT  COLON  <TypeExpr>  CLOSE_PAR
    { 10, { 23 } }, // [17]  <TypeExpr>  ==>  _Type
    { 10, { 5 } }, // [18]  <TypeExpr>  ==>  _Prop
    { 10, { 4 } }, // [19]  <TypeExpr>  ==>  IDENT
    { 10, { 26, -11, 29, -11, 27 } }, // [20]  <TypeExpr>  ==>  OPEN_PAR  <TypeExpr>  SLIM_ARROW  <TypeExpr>  CLOSE_PAR
    { 12, { 4, -15 } }, // [21]  <ArgExpr>  ==>  IDENT  <ArgList>
    { 12, { 18, -15 } }, // [22]  <ArgExpr>  ==>  _Eval  <ArgList>
    { 12, { 15, -15 } }, // [23]  <ArgExpr>  ==>  _Expression  <ArgList>
    { 12, { 6 } }, // [24]  <ArgExpr>  ==>  _Prop_True
    { 12, { 7 } }, // [25]  <ArgExpr>  ==>  _Prop_False
    { 14, { 26, -16, 27 } }, // [26]  <ArgList>  ==>  OPEN_PAR  <Args>  CLOSE_PAR
    { 14, {  } }, // [27]  <ArgList>  ==>
    { 15, { -13, -16 } }, // [28]  <Args>  ==>  <ArgExpr>  <Args>
    { 15, {  } }, // [29]  <Args>  ==>
    { 11, { -17, -12 } }, // [30]  <Statements>  ==>  <Statement>  <Statements>
    { 11, {  } }, // [31]  <Statements>  ==>
    { 16, { 10, 4, 25, -13 } }, // [32]  <Statement>  ==>  _Let  IDENT  EQUAL  <ArgExpr>
    { 16, { -13 } }, // [33]  <Statement>  ==>  <ArgExpr>
    { 16, { 11, 4, -18, 14 } }, // [34]  <Statement>  ==>  _Match  IDENT  <Cases>  _End
    { 16, { 20, 4, -19, 14 } }, // [35]  <Statement>  ==>  _Induction  IDENT  <ICases>  _End
    { 16, { 19 } }, // [36]  <Statement>  ==>  _Trivial
    { 16, { 21, 4, -13, 30, -13 } }, // [37]  <Statement>  ==>  _Rewrite  IDENT  <ArgExpr>  FAT_ARROW  <ArgExpr>
    { 17, { -20, -18 } }, // [38]  <Cases>  ==>  <Case>  <Cases>
    { 17, { 13, 30, -12 } }, // [39]  <Cases>  ==>  _Else  FAT_ARROW  <Statements>
    { 17, {  } }, // [40]  <Cases>  ==>
    { 19, { 12, -13, 30, -12 } }, // [41]  <Case>  ==>  _Case  <ArgExpr>  FAT_ARROW  <Statements>
    { 18, { 12, -13, -21, -19 } }, // [42]  <ICases>  ==>  _Case  <ArgExpr>  <ICase>  <ICases>
    { 18, {  } }, // [43]  <ICases>  ==>
    { 20, { 4, 30, -12 } }, // [44]  <ICase>  ==>  IDENT  FAT_ARROW  <Statements>
    { 20, { 30, -12 } } // [45]  <ICase>  ==>  FAT_ARROW  <Statements>
}; // End of g_Productions



const ParsingTable Proof_Parser::g_ParsingTable = {
//END_O,COMME,SPACE,   NL,IDENT,_Prop,_Prop,_Prop,_Regi, _Def, _Let,_Matc,_Case,_Else, _End,_Expr,_Theo,_Proo,_Eval,_Triv,_Indu,_Rewr,_Indu,_Type,COLON,EQUAL,OPEN_,CLOSE,COLON,SLIM_,FAT_A, PIPE
      0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    0,    0,   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [0] <>
      1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    1,    1,   -1,   -1,   -1,   -1,   -1,   -1,    1,   -1,   -1,   -1,   -1,   -1,    1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [1] <Start>
      6,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    4,    3,   -1,   -1,   -1,   -1,   -1,   -1,    5,   -1,   -1,   -1,   -1,   -1,    2,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [2] <Item>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [3] <Inductive>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   11,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [4] <Def>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   12,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [5] <RegisterEquality>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   13,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [6] <Theorem>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    9,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    8, // [7] <ConstructorList>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   10, // [8] <Constructor>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   15,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   14,   -1,   15,   -1,   -1,   15, // [9] <ParamList>
     -1,   -1,   -1,   -1,   19,   18,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   17,   -1,   -1,   20,   -1,   -1,   -1,   -1,   -1, // [10] <TypeExpr>
     -1,   -1,   -1,   -1,   30,   -1,   30,   30,   -1,   -1,   30,   30,   31,   31,   31,   30,   -1,   -1,   30,   30,   30,   30,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [11] <Statements>
     -1,   -1,   -1,   -1,   21,   -1,   24,   25,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   23,   -1,   -1,   22,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [12] <ArgExpr>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   16,   -1,   -1,   -1,   -1,   -1, // [13] <Param>
     -1,   -1,   -1,   -1,   27,   -1,   27,   27,   -1,   -1,   27,   27,   27,   27,   27,   27,   -1,   27,   27,   27,   27,   27,   -1,   -1,   -1,   -1,   26,   27,   -1,   -1,   27,   -1, // [14] <ArgList>
     -1,   -1,   -1,   -1,   28,   -1,   28,   28,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   28,   -1,   -1,   28,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   29,   -1,   -1,   -1,   -1, // [15] <Args>
     -1,   -1,   -1,   -1,   33,   -1,   33,   33,   -1,   -1,   32,   34,   -1,   -1,   -1,   33,   -1,   -1,   33,   36,   35,   37,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [16] <Statement>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   38,   39,   40,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [17] <Cases>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   42,   -1,   43,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [18] <ICases>
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // [19] <Case>
     -1,   -1,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   45,   -1  // [20] <ICase>
}; // End of g_ParsingTable
