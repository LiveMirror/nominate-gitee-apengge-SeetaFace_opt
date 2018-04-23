#ifndef __ALOCATOR_H__
#define __ALOCATOR_H__

#include "common.h"

class  BaseMemAllocator
{
public:
	SEETA_API BaseMemAllocator();
	SEETA_API virtual ~BaseMemAllocator();
	virtual void * malloc(int len) = 0;
	virtual void free(void * p) = 0;
};

extern  BaseMemAllocator * gMemAllocator;

void SEETA_API setGMemAllocator(BaseMemAllocator * allocator);

class  SysMemAllocator : public BaseMemAllocator
{
public:
	SysMemAllocator();
	virtual ~SysMemAllocator();
	virtual void * malloc(int len);
	virtual void free(void * p);
};




#endif