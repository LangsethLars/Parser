#include <cstdlib>
#include <cstring>

#include "BitSet.h"



BitSet::BitSet(size_t nOfBits) :
	m_MemPtr(0), m_MemCapacity(0)
{
	if (nOfBits)
		initBitSet(nOfBits);
}



BitSet::BitSet(const BitSet &rhs) :
	m_MemPtr(0), m_MemCapacity(0)
{
	if (rhs.m_MemCapacity > 0) {
		m_MemCapacity = rhs.m_MemCapacity;
		m_MemPtr = (unsigned char *)malloc(m_MemCapacity);
		memcpy(m_MemPtr, rhs.m_MemPtr, rhs.m_MemCapacity);
	}
}



BitSet &
BitSet::operator=(const BitSet &rhs)
{
	if (this != &rhs) {
		m_MemCapacity = rhs.m_MemCapacity;
		if (m_MemCapacity == 0) {
			free(m_MemPtr);
			m_MemPtr = 0;
		} else {
			if (m_MemPtr)
				m_MemPtr = (unsigned char *)realloc(m_MemPtr, m_MemCapacity);
			else
				m_MemPtr = (unsigned char *)malloc(m_MemCapacity);
			memcpy(m_MemPtr, rhs.m_MemPtr, m_MemCapacity);
		}
	}
	return *this;
}


BitSet::~BitSet()
{
	free(m_MemPtr);
}



BitSet &
BitSet::clear()
{
	if ( m_MemPtr && m_MemCapacity)
		memset(m_MemPtr, 0, m_MemCapacity);

	return *this;
}



void
BitSet::initBitSet(size_t nOfBits)
{
	m_MemCapacity = (nOfBits + 7) >> 3;
	if (m_MemCapacity) {
		if (m_MemPtr)
			m_MemPtr = (unsigned char *)realloc(m_MemPtr, m_MemCapacity);
		else
			m_MemPtr = (unsigned char *)malloc(m_MemCapacity);
		memset(m_MemPtr, 0, m_MemCapacity);
	} else {
		free(m_MemPtr);
		m_MemPtr = 0;
	}
}



bool
BitSet::orWith(const BitSet &rhs)
{
	bool hasChanged = false;

	unsigned char *p = m_MemPtr;
	unsigned char *pEnd = m_MemPtr + m_MemCapacity;
	unsigned char *q = rhs.m_MemPtr;

	while (p < pEnd) {
		unsigned char ch = *p;
		*p |= *q;
		hasChanged = hasChanged || ch != *p;
		++p; ++q;
	}

	return hasChanged;

}
