#pragma once
#include <cstdint>
#include <array>

class ChSet {

public:

	ChSet() : m_ChSet{} { }  // Zero-initialization

	void clear() { m_ChSet.fill(0); }

	void addChar(uint8_t ch) { m_ChSet[ch / 8] |= 1 << (ch % 8); }
	void removeChar(uint8_t ch) { m_ChSet[ch / 8] &= ~(1 << (ch % 8)); }
	void addChars(const uint8_t* pStr) { while (*pStr) addChar(*pStr++); }

	void addSet(const ChSet &rhs) { 
		for (size_t i = 0; i < m_ChSet.size(); ++i) 
			m_ChSet[i] |= rhs.m_ChSet[i]; 
	}
	
	void removeSet(const ChSet &rhs) { 
		for (size_t i = 0; i < m_ChSet.size(); ++i) 
			m_ChSet[i] &= ~(rhs.m_ChSet[i]); 
	}

	void addRange(int c0, int cN) { while (c0 <= cN) addChar(c0++); }
	void removeRange(int c0, int cN) { while (c0 <= cN) removeChar(c0++); }

	bool isCharInSet(uint8_t ch) const { return m_ChSet[ch / 8] & (1 << (ch % 8)); }

private:

	std::array<uint8_t, 32> m_ChSet;

};  // End of class ChSet

