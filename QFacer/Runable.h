
#ifndef __RUNABLE_H__
#define __RUNABLE_H__

#include "common.h"
#include "Logger.h"


typedef int(*RunableFun)(void *);

typedef enum
{
	RunableStateStoped = 0, 
	RunableStateStoping, 
	RunableStateStarted
}RunableState;

class Runable
{
public:
	Runable();
	Runable(bool releaseSelfWhenExist);

	virtual ~Runable();

	void run();

	virtual int start() = 0;

	void stopAndWaitThreadQuit();

	void waitThreadQuit();

    HANDLE  theadHandle;

	RunableState state;

	bool releaseSelfWhenExist;	

	long threadId;
};

#endif //__RUNABLE_H__

