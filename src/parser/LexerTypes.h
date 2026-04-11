#pragma once

#include <string>
#include <vector>
#include <cstdint>



struct TokenClass {
	TokenClass(const char *tokenName, bool ignoreToken) 
		: name(tokenName), ignore(ignoreToken) { }
	
	std::string name;
	bool ignore;
};

using TokenClasses = std::vector<TokenClass>;



struct LexerState {
	int tokenClassId;  // If this is not an accepting state, then tokenClassId = -1
	int onChMove[256]; // 0...n is move to new state, -1 is no move -> check if accepting state
};

using LexerTable = std::vector<LexerState>;

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



using RawText = std::vector<uint8_t>;



struct Token {
	int tokenClassId;   // Id to TokenClass
	size_t lexemeStart;
	size_t lexemeLength;
	int lineNumber;     // For debugging
};

using TokenSequence = std::vector<Token>;
