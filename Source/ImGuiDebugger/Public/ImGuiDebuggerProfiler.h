#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImGuiDebuggerExtension.h"

class FImGuiDebuggerStats;

struct FGroupFilter : public IItemFilter
{
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
	void HandleNewFrame(int64 Frame);
	void HandleNewFrameGT();

	void GetStats();
	
private:
	FDelegateHandle OnNewFrameDelegateHandle;

};