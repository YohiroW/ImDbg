#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImDbgExtension.h"
#include "ImDbgGPUProfiler.h"
#include "ImDbgMemoryProfiler.h"

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

	virtual void ShowMenu(float InDeltaTime) override;

private:
	bool bShowCPUProfiler = false;
	bool bShowGPUProfiler = false;
	bool bShowMemoryProfiler = false;

	bool TraceChannels[EImDbgTraceChannel::Num];

	TSharedPtr<FImDbgMemoryProfiler> MemoryProfiler;
	TSharedPtr<FImDbgGPUProfiler> GPUProfiler;
};