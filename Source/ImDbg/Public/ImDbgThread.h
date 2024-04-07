#pragma once

#include "Engine/EngineBaseTypes.h"
#include "HAL/Runnable.h"

class FImDbgThread : public FRunnable
{
public:
    FImDbgThread();
    virtual ~FImDbgThread();

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;

    void StartThread();
    void StopThread();

    FEvent* GetWaitFrameEvent(int32 RequestedBackBufferIndex);

    bool IsThreadRunning() const;

private:
    FRunnableThread* Thread = nullptr;

    bool bIsThreadRunning = false;
	bool bRequestExit = false;
};