#include "GreedyMemManager.h"

LONGLONG  miniSecondsBettwen(SYSTEMTIME st1, SYSTEMTIME st2)
{
	union timeunion {
		FILETIME fileTime;
		ULARGE_INTEGER ul;
	};

	timeunion ft1;
	timeunion ft2;

	SystemTimeToFileTime(&st1, &ft1.fileTime);
	SystemTimeToFileTime(&st2, &ft2.fileTime);

	_int64 ret;
	if (ft1.ul.QuadPart > ft2.ul.QuadPart)
	{
		ret = (ft1.ul.QuadPart - ft2.ul.QuadPart) / 10000;
	}
	else
	{
		ret = -1 * ((ft2.ul.QuadPart - ft1.ul.QuadPart) / 10000);
	}

	//LOG_INFO(_T("miniSecondsBettwen st1:%hs, st2:%hs is %lld"), sayTime(st1).c_str(), sayTime(st2).c_str(), ret);

	return ret;

}
LONGLONG  secondsBettwen(SYSTEMTIME st1, SYSTEMTIME st2)
{
	return miniSecondsBettwen(st1, st2) / 1000;
}
GreedyMemManager gGreedyMemManager;

#define PRINT_MEM_LEAK_IDLE_SECONDS 20 //20秒没有任何申请和是否，据打印一次内存泄漏




GreedyMemManager::GreedyMemManager()
{

	GetLocalTime(&lastWorkTime);

    totalMallocCount = 0;
    
    blockReuseCount = 0;
	initManageSizes();


}


GreedyMemManager::~GreedyMemManager()
{

}




void * GreedyMemManager::malloc(int len)
{
	//csLock.lock();

	//void * ret = ::malloc(len);
	//leakInfos.insert(MEM_LEAK_INFOS_VALUE_TYPE(ret, MemLeakLog(len)));
	//GetLocalTime(&lastWorkTime);
	//csLock.unlock();

	//return ret;


	{
		int lenIndex = getLenIndex(len);
		if (lenIndex < 0)
		{
			byte* udata = (byte*)::malloc(len + 1);
			udata[0] = 0;
			return udata + 1;
		}
		else
		{

			totalMallocCount += 1;



			LenAndBlocks*  lenAndBLocks = &(manageSizes[lenIndex]);
			csLock.lock();
			SimpleList * readyBlocks = &(lenAndBLocks->blockArray);
			//GREEDY_READY_BLOCKS * usingBlocks = usingSize2BlockMaps.find(blockLen)->second;


			if (readyBlocks->len > 0)
			{
				blockReuseCount = blockReuseCount + 1;

				if (totalMallocCount > MAX_TOTAL_REUSE_COUNT)
				{
					totalMallocCount = totalMallocCount / 2;
					blockReuseCount = (blockReuseCount - 1) / 2 - 1; //少算一个谦虚一点，免得突破100%

				}

				GreedyMemBlock * block = (GreedyMemBlock *)readyBlocks->pop_front_node()->data;
				//usingBlocks->push_back(block);
				//allReadyBlockes.erase(block->nodeInAllReady);
				csLock.unlock();
				//LOG_INFO(_T("get one block with len:%ld from idle, current idle array size:%d,using array size:%d, total idle array size:%d， oldest using mem:%hs"), block->len, readyBlocks->size(), usingBlocks->size(), allReadyBlockes.size(), pointer2Str(usingBlocks->front()->p).c_str());
				return block->p;
			}
			else
			{
				if (totalMallocCount > MAX_TOTAL_REUSE_COUNT)
				{
					totalMallocCount = totalMallocCount / 2;
					blockReuseCount = (blockReuseCount - 1) / 2 - 1; //少算一个谦虚一点，免得突破100%

				}


				csLock.unlock();

				byte* udata = (byte*)::malloc(lenAndBLocks->len + 1 + sizeof(GreedyMemBlock *));
				byte* adata = udata + 1 + sizeof(GreedyMemBlock *);
				adata[-1] = 1;
				GreedyMemBlock *block = new GreedyMemBlock(-1, adata, lenIndex);

				((GreedyMemBlock **)udata)[0] = block;

				//LOG_INFO(_T("create new block with len:%ld from idle, current idle array size:0, using array size:%d, total idle array size:%d, oldest using mem:%hs"), block->len, usingBlocks->size(), allReadyBlockes.size(), pointer2Str(usingBlocks->front()->p).c_str());
				return block->p;
			}

		}
	}



	

}

void GreedyMemManager::free(void * p)
{

	//csLock.lock();

	//::free(p);
	//leakInfos.erase(leakInfos.find(p));
	//GetLocalTime(&lastWorkTime);
	//csLock.unlock();

	{
		byte *pTemp = (byte *)p;
		bool useSystemAllocator = (pTemp[-1] == 0);
		if (useSystemAllocator)
		{
			::free((pTemp - 1));
		}
		else
		{
			byte* udata = pTemp - 1 - sizeof(GreedyMemBlock *);

			GreedyMemBlock *block = ((GreedyMemBlock **)udata)[0];
			block->lastReadyTime = GetTickCount() / 1000; //当前秒数。
			csLock.lock();


			LenAndBlocks*  lenAndBLocks = &(manageSizes[block->lenIndex]);

			if (lenAndBLocks->blockArray.len >= GREEDY_KEEP_MAX_IDLE_BLOCKS_PER_LEN)
			{
				SafeDeleteObj(block);
			}
			else
			{
				lenAndBLocks->blockArray.push_back_node(block->nodeInReadyArray);
			}

			csLock.unlock();

			//LOG_INFO(_T("save one block with len:%ld as idle, current idle array size:%d,using array size:%d, total idle array size:%d"), block->len, readyBlocks->size(), usingBlocks->size(), allReadyBlockes.size());
		}
	}
}



int GreedyMemManager::getLenIndex(unsigned int mallocSize)
{
	if (mallocSize < GREEDY_MEM_MANAGE_MIN_BLOCK_SIZE || mallocSize > GREEDY_MEM_MANAGE_MAX_BLOCK_SIZE)
	{
		return -1;
	}

	int ret = -1;
	for (int i = 0; i < MANAGER_MEM_SIZES_COUNT; i++)
	{
		if (manageSizes[i].len <= 0)
		{
			break;
		}
		else if (manageSizes[i].len < mallocSize)
		{
			//coninue
		}
		else
		{

			ret = i;
			break;
		}
	}

	return ret;;
}

LenAndBlocks* GreedyMemManager::getLenAndBLocks(unsigned int mallocSize)
{
	LenAndBlocks * ret = NULL;

	if (mallocSize < GREEDY_MEM_MANAGE_MIN_BLOCK_SIZE || mallocSize > GREEDY_MEM_MANAGE_MAX_BLOCK_SIZE)
	{
		return ret;
	}

	for (int i = 0; i < MANAGER_MEM_SIZES_COUNT; i++)
	{
		if (manageSizes[i].len <= 0)
		{
			break;
		}
		else if (manageSizes[i].len < mallocSize)
		{
			//coninue
		}
		else
		{
			ret = &manageSizes[i];
			break;
		}
	}


	return ret;;
}


double GreedyMemManager::reuseBlockHitRate()
{
	long totalMallocCountTemp = totalMallocCount;

	long blockReuseCountTemp = blockReuseCount;

	if (totalMallocCount == 0)
	{
		return 0.0;
	}
	else
	{
		return blockReuseCountTemp*1.0 / totalMallocCountTemp;
	}
}


void GreedyMemManager::cleanReuseBlockHitRateData()
{
	totalMallocCount = 0;

	blockReuseCount = 0;
}

void GreedyMemManager::initManageSizes()
{
	for (int i = 0; i < MANAGER_MEM_SIZES_COUNT; i++)
	{
		manageSizes[i].len = -1;
	}

	//readySize2BlockMaps.clear();
	int iSiseIndex = 0;
	unsigned int size = GREEDY_MEM_MANAGE_MIN_BLOCK_SIZE;
	while (true)
	{
		manageSizes[iSiseIndex].len = size; iSiseIndex++;
		//readySize2BlockMaps.insert(GREEDY_READY_SIZE_BLOCK_MAP_VALUE_TYPE(size, new GREEDY_READY_BLOCKS()));
		//usingSize2BlockMaps.insert(GREEDY_READY_SIZE_BLOCK_MAP_VALUE_TYPE(size, new GREEDY_READY_BLOCKS()));

		unsigned int size2 = (size / 4) * 7;
		if (size2 > GREEDY_MEM_MANAGE_MAX_BLOCK_SIZE)
		{
			break;
		}
		else
		{
			//readySize2BlockMaps.insert(GREEDY_READY_SIZE_BLOCK_MAP_VALUE_TYPE(size2, new GREEDY_READY_BLOCKS()));
			//usingSize2BlockMaps.insert(GREEDY_READY_SIZE_BLOCK_MAP_VALUE_TYPE(size2, new GREEDY_READY_BLOCKS()));
			manageSizes[iSiseIndex].len = size; iSiseIndex++;
		}

		size = size * 2;
		if (size > GREEDY_MEM_MANAGE_MAX_BLOCK_SIZE)
		{
			break;
		}
		else
		{
			//continue
		}
	}




}
