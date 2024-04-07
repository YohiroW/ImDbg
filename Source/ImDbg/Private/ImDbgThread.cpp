#include "ImDbgThread.h"

FImDbgThread::FImDbgThread()
{
   
}

FImDbgThread::~FImDbgThread()
{
    StopThread();
}

bool FImDbgThread::Init()
{
    return true;
}

uint32 FImDbgThread::Run()
{
	static int32 Counter = 0;
	while (bIsThreadRunning)
	{

		if (bRequestExit)
		{
			break;
		}
    }

    return 0;
}

void FImDbgThread::Stop()
{
    bIsThreadRunning = false;
}

void FImDbgThread::Exit()
{
	bRequestExit = true;
}

void FImDbgThread::StartThread()
{
	bIsThreadRunning = true;

	Thread = FRunnableThread::Create(this, TEXT("ImDbgThread"), 0, TPri_Normal, FPlatformAffinity::GetPoolThreadMask()); 
}

void FImDbgThread::StopThread()
{
    if (Thread)
    {
        Thread->Kill(true);
        delete Thread;
        Thread = nullptr;
    }
}

FEvent* FImDbgThread::GetWaitFrameEvent(int32 RequestedBackBufferIndex)
{
    return nullptr;
}

bool FImDbgThread::IsThreadRunning() const
{
    return bIsThreadRunning;
}