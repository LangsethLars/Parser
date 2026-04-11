#include "BitSet.h"



BitSet::BitSet(size_t nOfBits) :
	m_MemBuffer()
{
	if (nOfBits > 0)
		initBitSet(nOfBits);
}



BitSet &
BitSet::clear()
{
	std::fill(m_MemBuffer.begin(), m_MemBuffer.end(), 0);
	return *this;
}



void
BitSet::initBitSet(size_t nOfBits)
{
	size_t capacity = (nOfBits + 7) >> 3;
	m_MemBuffer.resize(capacity, 0);  // Resize and zero-initialize
}



bool
BitSet::orWith(const BitSet &rhs)
{
	bool hasChanged = false;

	size_t minSize = std::min(m_MemBuffer.size(), rhs.m_MemBuffer.size());
	
	for (size_t i = 0; i < minSize; ++i) {
		uint8_t oldValue = m_MemBuffer[i];
		m_MemBuffer[i] |= rhs.m_MemBuffer[i];
		hasChanged = hasChanged || (oldValue != m_MemBuffer[i]);
	}

	return hasChanged;
}
