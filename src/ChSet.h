#pragma once
#include <cstring>

class ChSet {

public:

	ChSet() { memset(m_ChSet, 0, 32); }

	void clear() { memset(m_ChSet, 0, 32); }

	void addChar(unsigned char ch) { m_ChSet[ch / 8] |= 1 << (ch % 8); }
	void removeChar(unsigned char ch) { m_ChSet[ch / 8] &= ~(1 << (ch % 8)); }
	void addChars(const char *pStr) { while (*pStr) addChar((unsigned char)(*pStr++)); }

	void addSet(ChSet &rhs) { for (int i = 0; i < 32; ++i) m_ChSet[i] |= rhs.m_ChSet[i]; }
	void removeSet(ChSet &rhs) { for (int i = 0; i < 32; ++i) m_ChSet[i] &= ~(rhs.m_ChSet[i]); }

	void addRange(int c0, int cN) { while (c0 <= cN) addChar(c0++); }
	void removeRange(int c0, int cN) { while (c0 <= cN) removeChar(c0++); }

	bool isCharInSet(unsigned char ch) { return m_ChSet[ch / 8] & (1 << (ch % 8)); }

private:

	unsigned char m_ChSet[32];

};  // End of class ChSet

