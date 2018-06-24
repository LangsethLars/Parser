#pragma once

#include <string>
#include <vector>



struct TokenClass {
	TokenClass(const char *tokenName, bool ignoreToken) : name(tokenName), ignore(ignoreToken) { }
	std::string name;
	bool ignore;
};

typedef std::vector<TokenClass> TokenClasses;



struct VariableClass {
	VariableClass(const char *variableName, bool skipToken = false) : name(variableName), skip(skipToken) { }
	std::string name;
	bool skip;
};

typedef std::vector<VariableClass> VariableClasses;



struct LexerState {
	// void init(int id = -1) { tokenClassId = id; memset(onChMove, 255, sizeof(onChMove)); }
	int tokenClassId;  // If this is not an accepting state, then tokenClassId = -1
	int onChMove[256]; // 0...n is move to new state, -1 is no move -> check if accepting state
};

typedef std::vector<LexerState> LexerTable;

// Example with two token definitions:
// -----------------------------------
// TokenClassId 1: alpha alphanum_*
// TokenClassId 2: "if"
//
//     id  onChMove[]
//    ---+-------------------------------------
//  0: -1  'i' -> 1, alpha     -> 3, rest -> -1
//  1:  1  'f' -> 2, alphanum_ -> 3, rest -> -1
//  2:  1            alphanum_ -> 3, rest -> -1
//  3:  2            alphanum_ -> 3, rest -> -1



struct Production {
	int variableClassId;
	std::vector<int> symbols; // -(n+1) means variableClassId #n, else the entry is tokenClassId
};

typedef std::vector<Production> Productions;

// Example with two productions:
// -----------------------------
// <>      = <Start> END_OF_FILE
// <Start> = SEMICOLON
//
// {
//    { 0, { -2, 0 } },  // <>      has id = 0 , { -2 = -(1+1) variableClassId 1 is <Start>, 0 = tokenClassId for END_OF_FILE }
//    { 1, { 1 } }       // <Start> has id = 1 , { 1 = tokenClassId for COMMENT }
// }

typedef std::vector<int> ParsingTable;   // ParsingTable[ #VariableClasses , #TokenClasses ] 2D table pointing to production #



typedef std::vector<unsigned char> RawText;



struct Token {
	int tokenClassId;   // Id to TokenClass
	size_t lexemeStart;
	size_t lexemeLength;
	int lineNumber;     // For debugging
	//std::string string; // Characters in input that makes up this token
};

typedef std::vector<Token> TokenSequence;



struct ParseNode {

//	int tokenNumber;     // token number in sequence of tokens, -1 means it is a VariableClass
//	int variableClassId; // Id to VariableClass, -1 means it is a token/terminal
	int symbolIdOrTokenSequenceNo; // -(n+1) means variableClassId #n, else token number in sequence of tokens

	int parent;
	int firstChild;
	int lastChild;

	int prev; // -1 ==> First symbol of this local level
	int next; // -1 ==> Last symbol of this local level

};

typedef std::vector<ParseNode> ParseTree;



class Parser {

public:

	Parser(const TokenClasses &tc, const VariableClasses &vc, const LexerTable &lt, const Productions &prod, const ParsingTable &pt);

	bool lexRawText();
	bool lexAndParseRawText();

	void displayErrorMessage(const unsigned char *pTokenStart, const unsigned char *pTokenErr, int tokenLineNumber);
	void debug();

private:

	int getTokenClassId(const char *name);
	int getVariableClassId(const char *name);

public:

	// Fixed token/variable classes
	const TokenClasses &m_TokenClasses;
	const VariableClasses &m_VariableClasses;

private:

	// Fixed table driven lexer and parser
	const LexerTable &m_LexerTable;
	const Productions &m_Productions;
	const ParsingTable &m_ParsingTable;

public:

	// Result from lex() and lexAndParse()
	RawText m_RawText;
	TokenSequence m_TokenSequence;	// Both lex() and lexAndParse()
	ParseTree m_ParseTree;			// Only from lexAndParse()

};



class NodeIter {

public:

	NodeIter(Parser &p, int n = 0) : m_ParserPtr(&p), m_NParseTree(n) { }
	NodeIter(const NodeIter &rhs) : m_ParserPtr(rhs.m_ParserPtr), m_NParseTree(rhs.m_NParseTree) { }
	NodeIter &operator=(const NodeIter &rhs) { m_ParserPtr = rhs.m_ParserPtr; m_NParseTree = rhs.m_NParseTree; }

	bool operator==(const NodeIter &rhs) { return (m_ParserPtr == rhs.m_ParserPtr) && (m_NParseTree == rhs.m_NParseTree); }
	bool operator!=(const NodeIter &rhs) { return (m_ParserPtr != rhs.m_ParserPtr) || (m_NParseTree != rhs.m_NParseTree); }

	NodeIter parent() { NodeIter it(*m_ParserPtr, m_ParserPtr->m_ParseTree[m_NParseTree].parent); return it; }

	bool hasChildren() { return m_ParserPtr->m_ParseTree[m_NParseTree].firstChild >= 0; }
	NodeIter firstChild() { NodeIter it(*m_ParserPtr, m_ParserPtr->m_ParseTree[m_NParseTree].firstChild); return it; }
	NodeIter lastChild() { NodeIter it(*m_ParserPtr, m_ParserPtr->m_ParseTree[m_NParseTree].lastChild); return it; }

	NodeIter next() { NodeIter it(*m_ParserPtr, m_ParserPtr->m_ParseTree[m_NParseTree].next); return it; }
	NodeIter prev() { NodeIter it(*m_ParserPtr, m_ParserPtr->m_ParseTree[m_NParseTree].prev); return it; }

	NodeIter &operator++() { m_NParseTree = m_ParserPtr->m_ParseTree[m_NParseTree].next; return *this; }

	ParseNode &getNode() { return m_ParserPtr->m_ParseTree[m_NParseTree]; }

	int symbol() {
		int n = getNode().symbolIdOrTokenSequenceNo;
		return n < 0 ? n : m_ParserPtr->m_TokenSequence[n].tokenClassId;
	}

	Token &getToken() { return m_ParserPtr->m_TokenSequence[getNode().symbolIdOrTokenSequenceNo]; }

	std::string lexeme() {
		Token &token = getToken();
		return std::string((const char *)&(m_ParserPtr->m_RawText[token.lexemeStart]), token.lexemeLength);
	}

	void debug(const char *msg);

private:

	Parser *m_ParserPtr;
	int m_NParseTree;

};
