#include "allocator.h"
#include "stdlib.h"


SysMemAllocator gSysMemAllocator;
BaseMemAllocator * gMemAllocator = &gSysMemAllocator;


BaseMemAllocator::BaseMemAllocator(){};
BaseMemAllocator::~BaseMemAllocator(){};

SysMemAllocator::SysMemAllocator(){};
SysMemAllocator::~SysMemAllocator(){};
void * SysMemAllocator::malloc(int len)
{
	return ::malloc(len);

}

void SysMemAllocator::free(void * p)
{
	return ::free(p);
}


void setGMemAllocator(BaseMemAllocator * allocator)
{
	gMemAllocator = allocator;
}