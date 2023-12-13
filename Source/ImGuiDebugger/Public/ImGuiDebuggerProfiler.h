#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImGuiDebuggerExtension.h"

class FImGuiDebuggerStats;

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
	FStatsFetchThread(FImGuiDebuggerStats& InStatsDebugger);
	~FStatsFetchThread();

	virtual uint32 Run() override;

	virtual void Stop() override;

	void StartThread();

protected:
	FRunnableThread* RunnableThread;

	FImGuiDebuggerStats& StatsDebugger;

	FThreadSafeBool bForceStop;
};

class FImGuiDebuggerStats : public FImGuiDebuggerExtension
{
public:
	FImGuiDebuggerStats();
	~FImGuiDebuggerStats();

	virtual void ShowMenu() override;

	void AddNewFrameDelegate();
	void RemoveNewFrameDelegate();

	void HandleNewFrame(int64 Frame);
	void HandleNewFrameGT();

	void StartCollectPerfData();
	void GetStats();
	void StopCollectPerfData();
	
private:
	FDelegateHandle OnNewFrameDelegateHandle;

	bool bIsCollecting = false;
};

class FImGuiDebuggerGPUProfiler : public FImGuiDebuggerExtension
{
public:
	FImGuiDebuggerGPUProfiler();
	~FImGuiDebuggerGPUProfiler();

	virtual void ShowMenu() override;

	void InitializeStats();


private:
	FImGuiDebuggerStats Stats;

};