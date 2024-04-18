#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImDbgExtension.h"

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

class FImDbgGPUProfiler : public FImDbgExtension
{
public:
	enum EGPUProfilerType
	{
		Frame,
		RenderThread,
		RHI,
		GPU,
		GPUProfiler_Count
	};

	const char* GPUProfilerTypeText[EGPUProfilerType::GPUProfiler_Count] =
	{
		"Frame",
		"RenderThread",
		"RHI",
		"GPU"
	};

public:
	FImDbgGPUProfiler(bool* bInEnabled);
	~FImDbgGPUProfiler();

	virtual void ShowMenu() override;
	void ShowGPUGeneral();

	void Initialize();
	void RegisterDelegate();
	void UnRegisterDelegate();
	bool IsRegistered() const { return bIsRegistered; }
	void OnHandleNewFrame(int64 Frame);

private:
	//FImDbgStats Stats;
	bool* bEnabled;
	bool IsGPUProfilerEnabled[GPUProfiler_Count];
	bool bIsRegistered = false;
	FDelegateHandle OnNewFrameDelegateHandle;

	TMap<FName, double> GPUStats;
};