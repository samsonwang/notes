#ifndef SIMPLEBUFFER_H_
#define SIMPLEBUFFER_H_

#include "ostype.h"

class CSimpleBuffer
{
public:
	CSimpleBuffer();
	~CSimpleBuffer();
	uchar_t*  GetBuffer();
	uint32_t GetAllocSize();
	uint32_t GetWriteOffset();
	void IncWriteOffset(uint32_t len);

	void Extend(uint32_t len);
	uint32_t Write(void* buf, uint32_t len);
	uint32_t Read(void* buf, uint32_t len);
private:
	uchar_t*	m_buffer;
	uint32_t	m_alloc_size;
	uint32_t	m_write_offset;
};
#endif /* SIMPLEBUFFER_H_ */
