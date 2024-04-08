#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImDbgExtension.h"

class FImDbgStats;

struct FImDbgGeneralStats
{
	float FrameTime;
	float FPS;
	float GameThreadTime;
	float RenderThreadTime;
	//float AudioThreadTime;
	//float ImGuiThreadTime;
	float GPUTime;
	float SwapBufferTime;
	float RHIThreadTime;
	float InputLatency;
};

struct FGroupFilter 
#if STATS
	: public IItemFilter
#endif
{
#if STATS
	const TSet<FName>& EnabledItems;

	FGroupFilter(const TSet<FName>& InEnabledItems)
		: EnabledItems(InEnabledItems)
	{
	}

	virtual bool Keep(const FStatMessage& Item)
	{
		const FName MessageName = Item.NameAndInfo.GetRawName();
		return EnabledItems.Contains(MessageName);
	}
#endif
};

class FStatsFetchThread : public FRunnable
{
public:
	FStatsFetchThread(FImDbgStats& InStatsDebugger);
	~FStatsFetchThread();

	virtual uint32 Run() override;

	virtual void Stop() override;

	void StartThread();

protected:
	FRunnableThread* RunnableThread;

	FImDbgStats& StatsDebugger;

	FThreadSafeBool bForceStop;
};

class FImDbgStats : public FImDbgExtension
{
public:
	FImDbgStats();
	~FImDbgStats();

	virtual void ShowMenu() override;

	void AddNewFrameDelegate();
	void RemoveNewFrameDelegate();

	void HandleNewFrame(int64 Frame);
	void HandleNewFrameGT();

	void StartCollectPerfData();
	void GetStats();
	void StopCollectPerfData();

	FImDbgGeneralStats GetImDbgStats() { return Stats; }
	float GetFrameTime()        const { return Stats.FrameTime; }
	float GetFPS()              const { return Stats.FPS; }
	float GetGameThreadTime()   const { return Stats.GameThreadTime; }
	float GetRenderThreadTime() const { return Stats.RenderThreadTime; }
	float GetGPUTime()          const { return Stats.GPUTime; }
	float GetSwapBufferTime()   const { return Stats.SwapBufferTime; }
	float GetRHIThreadTime()    const { return Stats.RHIThreadTime; }
	float GetInputLatency()     const { return Stats.InputLatency; }
	
private:
	FDelegateHandle OnNewFrameDelegateHandle;
	bool bIsCollecting = false;

	FImDbgGeneralStats Stats;
};

class FImDbgMemoryProfiler : public FImDbgExtension
{
public:
	FImDbgMemoryProfiler(bool* bInEnabled);
	~FImDbgMemoryProfiler();

	virtual void ShowMenu() override;

private:
	bool* bEnabled;
};

class FImDbgGPUProfiler : public FImDbgExtension
{
public:
	FImDbgGPUProfiler(bool* bInEnabled);
	~FImDbgGPUProfiler();

	virtual void ShowMenu() override;

	void InitializeStats();


private:
	FImDbgStats Stats;
	bool* bEnabled;
};

class FImDbgProfiler : public FImDbgExtension
{
public:
	FImDbgProfiler();
	~FImDbgProfiler();

	void Initialize();

	virtual void ShowMenu() override;

private:
	bool bShowCPUProfiler = false;
	bool bShowGPUProfiler = false;
	bool bShowMemoryProfiler = false;

	TSharedPtr<FImDbgMemoryProfiler> MemoryProfiler;
	TSharedPtr<FImDbgGPUProfiler> GPUProfiler;

};