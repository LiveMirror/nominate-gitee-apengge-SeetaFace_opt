
#include "Runable.h"

DWORD WINAPI startRunable(LPVOID p)
{
    LOG_INFO("sub thread start");
	Runable * self = (Runable *)p;
	self->threadId = GetCurrentThreadId();
    self->state = RunableStateStarted;
	self->start();
	self->threadId = -1;
	Sleep(50);
	self->state = RunableStateStoped;
	
	if (self->releaseSelfWhenExist)
	{
		SafeDeleteObj(self);
	}
    LOG_INFO("sub thread end");
	return f_success;
}

Runable::Runable()
{
	theadHandle = NULL;
	state = RunableStateStoped;
	releaseSelfWhenExist = false;
	this->threadId;
}

Runable::Runable(bool releaseSelfWhenExist)
{
	this->releaseSelfWhenExist = releaseSelfWhenExist;
	state = RunableStateStoped;
	this->theadHandle = NULL;
	this->threadId = -1;
}


Runable::~Runable()
{
	if (theadHandle)
	{
		CloseHandle(theadHandle);
	}
	
	//theadHandle = NULL;
}


void Runable::run()
{
	theadHandle = CreateThread(NULL, 0, startRunable, this, 0, NULL);
	SuspendThread(theadHandle);        //暂停（挂起）线程
	SetThreadPriority(theadHandle, THREAD_PRIORITY_NORMAL); //设定线程的优先级别
	ResumeThread(theadHandle);           //继续运行线程
}


void Runable::stopAndWaitThreadQuit()
{
	if (state == RunableStateStoped)
	{
        LOG_WARN("this thread has stopped, no need to stop again");
		return;
	}
	else
	{
		state = RunableStateStoping;
		WaitForSingleObject(theadHandle, INFINITE);
	}
}

void Runable::waitThreadQuit() 
{
	WaitForSingleObject(theadHandle, INFINITE);
}
