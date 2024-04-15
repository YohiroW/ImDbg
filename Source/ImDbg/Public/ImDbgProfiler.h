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
	struct FTextureViewInfo
	{
		FString TextureName;
		FVector2D Dimension;
		float InMemSize;
		EPixelFormat PixelFormat;
		int32 GroupIndex;
		bool bIsStreaming;
		bool bIsVirtual;
		int32 UsageCount;
	};

	struct FRenderTargetViewInfo
	{
		FString Name;
		FVector2D Dimension;
		float Size;
		int32 MipLevels;
		EPixelFormat PixelFormat;
		int32 UnusedFrames;
	};

public:
	FImDbgMemoryProfiler(bool* bInEnabled);
	~FImDbgMemoryProfiler();

	virtual void ShowMenu() override;
	void ShowGeneralStatsView();
	void ShowTextureMemoryView();
	void ShowUObjectMermoryView();
	void ShowRenderTargetMemoryView();

	void UpdateTextureViewInfos();
	void UpdateRenderTargetViewInfos();

private:
	bool* bEnabled;

	bool bRequestUpdateTextureInfo = true;
	TArray<TSharedPtr<FTextureViewInfo>> TextureViewInfoList;

	bool bRequestUpdateRenderTargetInfo = true;
	TArray<TSharedPtr<FRenderTargetViewInfo>> RenderTargetViewInfoList;
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

	const char* GPUProfilerTypeText[5] =
	{
		"Frame",
		"RenderThread",
		"RHI",
		"GPU",
		"Invalid"
	};

public:
	FImDbgGPUProfiler(bool* bInEnabled);
	~FImDbgGPUProfiler();

	virtual void ShowMenu() override;

	void Initialize();

private:
	FImDbgStats Stats;
	bool* bEnabled;
	bool IsGPUProfilerEnabled[GPUProfiler_Count];
};

class FImDbgProfiler : public FImDbgExtension
{
public:	
	enum EImDbgTraceChannel
	{
		CPU,
		Frame,
		GPU,
		LoadTime,
		AssetLoadTime,
		Stats,
		Task,
		RDG,
		RHICommand,
		RenderCommands,
		Num
	};

	  const char* ImDbgTraceChannelName[EImDbgTraceChannel::Num] =
	  {
		  "CPU", "Frame", "GPU", "LoadTime", "AssetLoadTime", "Stats", "Task", "RDG", "RHICommand", "RenderCommands"
	  };

public:
	FImDbgProfiler();
	~FImDbgProfiler();

	void Initialize();

	virtual void ShowMenu() override;

private:
	bool bShowCPUProfiler = false;
	bool bShowGPUProfiler = false;
	bool bShowMemoryProfiler = false;

	bool TraceChannels[EImDbgTraceChannel::Num];

	TSharedPtr<FImDbgMemoryProfiler> MemoryProfiler;
	TSharedPtr<FImDbgGPUProfiler> GPUProfiler;
};