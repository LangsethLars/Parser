#pragma once

#include <deque>
#include <map>

#include "ChSet.h"
#include "BitSet.h"
#include "Parser.h"



// NFA, Nondeterministisk Finite Automaton

class BuildNFA {

public:

	BuildNFA &clear() { m_States.clear(); return *this; }

	// Basic API to create a stand alone single NFA

	BuildNFA &appendChSet(const ChSet &chSet);
	BuildNFA &appendCh(unsigned char ch);
	BuildNFA &appendStr(const char *pStr);
	BuildNFA &appendNFA(BuildNFA &rhsNFA);

	BuildNFA &this_or(BuildNFA &rhsNFA);

	BuildNFA &timesZeroOrOne();
	BuildNFA &timesZeroOrMore();
	BuildNFA &timesOneOrMore();

	// Used to create a full NFA with many start and accepting states
	void addToken(BuildNFA &newNFA, int tokenClassId);

	// Used to convert this NFA to a DFA
	void convertToDFA(LexerTable &lexerTable);

	void debug();

private:

	// Used to convert this NFA to a DFA
	void epsilonClosure(BitSet &nfaStates);
	void epsilonFollow(BitSet &nfaStates, size_t pos);
	int newStateDFA(LexerTable &lexerTable, BitSet &nfaStates, std::map<BitSet, int> &nfaStateMap);
	int getTokenClassId(BitSet &nfaStates);

private:

	struct State {

		State(bool isStart, int id, int m, int eM, ChSet chs) :
			isStartState(isStart), tokenClassId(id), move(m), epsilonMove(eM), chSet(chs)
		{}

		// Next two are only used when adding many single NFA to a collection of tokens, addToken().
		bool isStartState;	// Indicates the start of a token.
		int tokenClassId;	// If tokenClassId >= 0 then this is an accepting final state where other fields should be ignored.

		int move;			// Relative move, +1 = next state, 0 = same state, -1 = previous state
		int epsilonMove;	// If epsilonMove != 0: Ignore chSet and tokenClassId. Go to move and epsilonMove.
		ChSet chSet;		// If chSet.isCharInSet(character) goto move

	};

	std::deque<State> m_States;
	// Contains array of NFA State elements.
	//
	// Example with three token definitions:
	// -------------------------------------
	// tokenClassId 1: alpha alphanum_*
	// tokenClassId 2: "if"
	// tokenClassId 3: 'i'+ | 'b'
	//
	// s id  m eM chSet comment
	// -+--+--+--+-----+------------------------------
	// 1 -1 +1  0 alpha Start. On [A-Za-z] goto next.
	// 0 -1 +1 +2 ***** Epsilon goto next and two forward.
	// 0 -1 -1  0  an_  On [A-Za-z0-9_] goto previous.
	// 0  1 ** ** ***** Accepting tokenClassId = 1.
	//
	// 1 -1 +1  0  'i'  Start. On 'i' goto next.
	// 0 -1 +1  0  'f'  On 'f' goto next.
	// 0  2 ** ** ***** Accepting tokenClassId = 2.
	//
	// 1 -1 +1 +3 ***** Start. Epsilon goto next ('i'+) and three forward ('b').
	// 0 -1 +1  0  'i'  On 'i' goto next.
	// 0 -1 +2 -1 ***** Epsilon goto two forward and one back.
	// 0 -1 +1  0  'b'  On 'b' goto next.
	// 0  3 ** ** ***** Accepting tokenClassId = 3.

};  // End of class BuildNFA
