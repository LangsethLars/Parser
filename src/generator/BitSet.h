#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>



class BitSet {

public:

	BitSet(size_t nOfBits = 0);
	BitSet(const BitSet &rhs) = default;
	BitSet(BitSet &&rhs) noexcept = default;
	BitSet &operator=(const BitSet &rhs) = default;
	BitSet &operator=(BitSet &&rhs) noexcept = default;
	~BitSet() = default;

	BitSet &clear();
	size_t size() const { return m_MemBuffer.size(); }

	// API
	void initBitSet(size_t nOfBits);
	void setBit(size_t bitNo) { m_MemBuffer[bitNo >> 3] |= 1 << (bitNo & 7); }
	void clearBit(size_t bitNo) { m_MemBuffer[bitNo >> 3] &= ~(1 << (bitNo & 7)); }
	bool isBitSet(size_t bitNo) const { return (m_MemBuffer[bitNo >> 3] & (1 << (bitNo & 7))) != 0; }
	bool orWith(const BitSet &rhs); // Returns true if *this has changed

	// Compare operators so it can be organised in containers like std::map
	bool operator<  (const BitSet &rhs) const { return m_MemBuffer <  rhs.m_MemBuffer; }
	bool operator<= (const BitSet &rhs) const { return m_MemBuffer <= rhs.m_MemBuffer; }
	bool operator>  (const BitSet &rhs) const { return m_MemBuffer >  rhs.m_MemBuffer; }
	bool operator>= (const BitSet &rhs) const { return m_MemBuffer >= rhs.m_MemBuffer; }
	bool operator== (const BitSet &rhs) const { return m_MemBuffer == rhs.m_MemBuffer; }
	bool operator!= (const BitSet &rhs) const { return m_MemBuffer != rhs.m_MemBuffer; }

private:

	std::vector<uint8_t> m_MemBuffer;

};

