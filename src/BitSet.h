#pragma once

#include <cstdlib>
#include <cstring>



class BitSet {

public:

	BitSet(size_t nOfBits = 0);
	BitSet(const BitSet &rhs);
	BitSet &operator=(const BitSet &rhs);
	~BitSet();

	BitSet &clear();
	size_t size() const { return m_MemCapacity; }

	// API
	void initBitSet(size_t nOfBits);
	void setBit(size_t bitNo) { m_MemPtr[bitNo >> 3] |= 1 << (bitNo & 7); }
	void clearBit(size_t bitNo) { m_MemPtr[bitNo >> 3] &= ~(1 << (bitNo & 7)); }
	bool isBitSet(size_t bitNo) { return (m_MemPtr[bitNo >> 3] & (1 << (bitNo & 7))) != 0; }
	bool orWith(const BitSet &rhs); // Returns true if *this has changed

	// Compare operators so it can be organised in containers like std::map
	bool operator<  (const BitSet &rhs) const { size_t len = (m_MemCapacity <= rhs.m_MemCapacity) ? m_MemCapacity : rhs.m_MemCapacity; int cmp = memcmp(m_MemPtr, rhs.m_MemPtr, len); if (cmp == 0) return m_MemCapacity <  rhs.m_MemCapacity; else return cmp <  0; }
	bool operator<= (const BitSet &rhs) const { size_t len = (m_MemCapacity <= rhs.m_MemCapacity) ? m_MemCapacity : rhs.m_MemCapacity; int cmp = memcmp(m_MemPtr, rhs.m_MemPtr, len); if (cmp == 0) return m_MemCapacity <= rhs.m_MemCapacity; else return cmp <= 0; }
	bool operator>  (const BitSet &rhs) const { return rhs <  *this; }
	bool operator>= (const BitSet &rhs) const { return rhs <= *this; }
	bool operator== (const BitSet &rhs) const { size_t len = (m_MemCapacity <= rhs.m_MemCapacity) ? m_MemCapacity : rhs.m_MemCapacity; int cmp = memcmp(m_MemPtr, rhs.m_MemPtr, len); if (cmp == 0) return m_MemCapacity == rhs.m_MemCapacity; else return false; }

private:

	unsigned char *m_MemPtr;
	size_t         m_MemCapacity;

};

