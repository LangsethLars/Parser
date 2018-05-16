#include <map>
#include <cstdlib>
#include <cstdio>

#include "BuildNFA.h"
#include "BitSet.h"



BuildNFA &
BuildNFA::appendChSet(const ChSet &chSet)
{
	m_States.emplace_back(false, -1, 1, 0, chSet);

	return *this;
}



BuildNFA &
BuildNFA::appendCh(unsigned char ch)
{
	ChSet chSet;
	chSet.addChar(ch);
	appendChSet(chSet);

	return *this;
}



BuildNFA &
BuildNFA::appendStr(const char *pStr)
{
	while (*pStr)
		appendCh((unsigned char)(*pStr++));

	return *this;
}



BuildNFA &
BuildNFA::appendNFA(BuildNFA &rhsNFA)
{
	for (auto it = rhsNFA.m_States.begin(); it != rhsNFA.m_States.end(); ++it)
		m_States.emplace_back(*it);

	return *this;
}



BuildNFA &
BuildNFA::this_or(BuildNFA &rhsNFA)
{
	// Update move to "old end" in "old this" to new "end"
	int end = int(m_States.size());
	for (auto it = m_States.begin(); it != m_States.end(); ++it, --end) {
		if (it->move == end)
			it->move += rhsNFA.m_States.size();
		if (it->epsilonMove == end)
			it->epsilonMove += rhsNFA.m_States.size();
	}

	// Add an epsilon state at the "beginning"
	m_States.emplace_front(false, -1, 1, int(m_States.size()) + 1, ChSet());

	// Append the new states from rhsNFA
	for (auto it = rhsNFA.m_States.begin(); it != rhsNFA.m_States.end(); ++it)
		m_States.emplace_back(*it);

	return *this;
}



BuildNFA &
BuildNFA::timesZeroOrOne()
{
	// Add an epsilon state at the "beginning" pointing to "old this" and "new end"
	m_States.emplace_front(false, -1, 1, int(m_States.size()) + 1, ChSet());

	return *this;
}



BuildNFA &
BuildNFA::timesZeroOrMore()
{
	// Update move to "old end" to new epsilon state at the "beginning" instead
	int end = int(m_States.size());
	int newBeginning = -1;
	for (auto it = m_States.begin(); it != m_States.end(); ++it, --end, --newBeginning) {
		if (it->move == end)
			it->move = newBeginning;
		if (it->epsilonMove == end)
			it->epsilonMove = newBeginning;
	}

	// Add an epsilon state at the "beginning" pointing to "old this" and "new end"
	m_States.emplace_front(false, -1, 1, int(m_States.size()) + 1, ChSet());

	return *this;
}



BuildNFA &
BuildNFA::timesOneOrMore()
{
	// Add an epsilon state at the "end" pointing to "old this" and "end"
	m_States.emplace_back(false, -1, 1, -int(m_States.size()), ChSet());

	return *this;
}



void
BuildNFA::addToken(BuildNFA &newNFA, int tokenClassId)
{
	// Assume that newNFA is not empty
	// Append the new states from newNFA and mark the new start state
	auto it = newNFA.m_States.begin();
	m_States.emplace_back(*it);
	m_States.back().isStartState = true;
	for (++it; it != newNFA.m_States.end(); ++it)
		m_States.emplace_back(*it);

	// Add an acepting state at the "end" for the new token
	m_States.emplace_back(false, tokenClassId, 0, 0, ChSet());
}



void
BuildNFA::convertToDFA(LexerTable &lexerTable)
{
	std::map<BitSet, int> nfaStateMap;
	BitSet nfaStates(m_States.size());

	// Find all start states
	size_t stateNo = 0;
	for (auto it = m_States.begin(); it != m_States.end(); ++it, ++stateNo) {
		if (it->isStartState)
			nfaStates.setBit(stateNo);
	}

	// Make sure to include all epsilon moves
	epsilonClosure(nfaStates);

	// Recursively create DFA states from NFA
	lexerTable.clear();
	newStateDFA(lexerTable, nfaStates, nfaStateMap);
}



void
BuildNFA::epsilonClosure(BitSet &nfaStates)
{
	size_t stateNo = 0;
	for (auto it = m_States.begin(); it != m_States.end(); ++it, ++stateNo) {
		if (nfaStates.isBitSet(stateNo) && it->epsilonMove) {
			epsilonFollow(nfaStates, stateNo + it->move);
			epsilonFollow(nfaStates, stateNo + it->epsilonMove);
		}
	}
}



void
BuildNFA::epsilonFollow(BitSet &nfaStates, size_t stateNo)
{
	if (!nfaStates.isBitSet(stateNo)) {
		nfaStates.setBit(stateNo);
		State &state = m_States[stateNo];
		if (state.epsilonMove) {
			epsilonFollow(nfaStates, stateNo + state.move);
			epsilonFollow(nfaStates, stateNo + state.epsilonMove);
		}
	}
}



int
BuildNFA::newStateDFA(LexerTable &lexerTable, BitSet &nfaStates, std::map<BitSet, int> &nfaStateMap)
{
	// Add new state to the DFA and remember the corresponding BitSet
	int dfaStateNo = lexerTable.size();
	lexerTable.emplace_back();
	lexerTable[dfaStateNo].tokenClassId = getTokenClassId(nfaStates);
	nfaStateMap[nfaStates] = dfaStateNo;

	BitSet nextNfaStates(m_States.size());

	for (size_t ch = 0; ch < 256; ++ch) {

		nextNfaStates.clear();

		bool isTransitionOut = false; // If there are any moves for this ch
		size_t stateNo = 0;
		for (auto it = m_States.begin(); it != m_States.end(); ++it, ++stateNo) {
			if (nfaStates.isBitSet(stateNo) && it->epsilonMove == 0 && it->tokenClassId < 0 && it->chSet.isCharInSet(ch)) {
				isTransitionOut = true;
				nextNfaStates.setBit(stateNo + it->move);
			}
		}

		if (isTransitionOut) {
			epsilonClosure(nextNfaStates);
			auto p = nfaStateMap.find(nextNfaStates);
			if (p != nfaStateMap.end()) {
				// Move to previous dfa state for this ch
				lexerTable[dfaStateNo].onChMove[ch] = p->second;
			} else {
				// Move to new dfa state for this ch by creating a new DFA state
				lexerTable[dfaStateNo].onChMove[ch] = newStateDFA(lexerTable, nextNfaStates, nfaStateMap);
			}
		} else {
			// No move for this ch
			lexerTable[dfaStateNo].onChMove[ch] = -1;
		}

	}

	return dfaStateNo;
}



int
BuildNFA::getTokenClassId(BitSet &nfaStates)
{
	int tokenClassId = -1;

	size_t stateNo = 0;
	for (auto it = m_States.begin(); it != m_States.end(); ++it, ++stateNo) {
		if (nfaStates.isBitSet(stateNo) && it->tokenClassId > tokenClassId)
			tokenClassId = it->tokenClassId;
	}

	return tokenClassId;
}



void
BuildNFA::debug()
{
	printf("\n");
	printf("NFA used to create DFA\n");
	printf("----------------------\n");
	printf("n     = Index into State[] array\n");
	printf("S     = This is the starting state for a new token.\n");
	printf("id    = This is an accepting state for this token.\n");
	printf("move  = Used by chMap and eps. Relative move 1=next state, -1=previous state, ...\n");
	printf("eps   = Epsilon state: Ignore chMap, split into two paths and continue from relative position move and eps.\n");
	printf("chMap = If chMap.isCharInSet(ch) then use move to find next state else failed (not correct token)\n");
	printf("\n");
	printf("  n  S  id  move  eps chMap\n");
	printf("----+-+----+----+----+-----\n");
	int stateNo = 0;
	for (auto it = m_States.begin(); it != m_States.end(); ++it, ++stateNo) {
		printf("%4d %c ", stateNo, it->isStartState ? 'S' : ' ');
		if (it->tokenClassId < 0) {
			printf("     ");
			printf("%4d ", it->move + stateNo);
			if (it->epsilonMove) printf("%4d ", it->epsilonMove + stateNo); else printf("     ");
		} else {
			printf("%4d ", it->tokenClassId);
			printf("     ");
			printf("     ");
		}
		if (it->chSet.isCharInSet(9)) printf("\\09");
		if (it->chSet.isCharInSet(13)) printf("\\0D");
		if (it->chSet.isCharInSet(10)) printf("\\0A");
		if (it->chSet.isCharInSet(32)) printf("\\20");
		for (unsigned c = 33; c < 127; ++c) {
			if (it->chSet.isCharInSet(c)) printf("%c", c);
		}
		printf("\n");
		if (it->tokenClassId >= 0)
			printf("\n");
	}
}

