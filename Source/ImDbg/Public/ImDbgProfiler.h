#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImDbgExtension.h"

class FImDbgStats;

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
	
private:
	FDelegateHandle OnNewFrameDelegateHandle;

	bool bIsCollecting = false;
};

class FImDbgGPUProfiler : public FImDbgExtension
{
public:
	FImDbgGPUProfiler();
	~FImDbgGPUProfiler();

	virtual void ShowMenu() override;

	void InitializeStats();


private:
	FImDbgStats Stats;

};