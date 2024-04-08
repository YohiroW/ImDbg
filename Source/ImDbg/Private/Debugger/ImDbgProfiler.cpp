#include "ImDbgProfiler.h"
#include "ImDbgModule.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "Stats/StatsData.h"
#include <implot.h>

#define LOCTEXT_NAMESPACE "ImDbg"

#define IDB_PROFILER_CATRGORY "Profiler"

//
// The function below is taken from AutomationBlueprintFunctionLibrary.h
// To avoid add extra dependency
//
#if STATS
template <EComplexStatField::Type ValueType, bool bCallCount = false>
float HelperGetStat(FName StatName)
{
	if (FGameThreadStatsData* StatsData = FLatestGameThreadStatsData::Get().Latest)
	{
		if (const FComplexStatMessage* StatMessage = StatsData->GetStatData(StatName))
		{
			if (bCallCount)
			{
				return (float)StatMessage->GetValue_CallCount(ValueType);
			}
			else
			{
				return (float)FPlatformTime::ToMilliseconds64(StatMessage->GetValue_Duration(ValueType));
			}
		}
	}

#if WITH_EDITOR
	FText WarningOut = FText::Format(LOCTEXT("StatNotFound", "Could not find stat data for {0}, did you call ToggleStatGroup with enough time to capture data?"), FText::FromName(StatName));
	FMessageLog("PIE").Warning(WarningOut);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *WarningOut.ToString());
#endif

	return 0.f;
}
#endif

FImDbgStats::FImDbgStats()
{
	Stats.FrameTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	Stats.FPS = round(1000.0f / Stats.FrameTime);
	Stats.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	Stats.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	Stats.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
	Stats.RHIThreadTime = FPlatformTime::ToMilliseconds(GWorkingRHIThreadTime);
	Stats.InputLatency = FPlatformTime::ToMilliseconds(GInputLatencyTimer.DeltaTime);
	Stats.SwapBufferTime = FPlatformTime::ToMilliseconds(GSwapBufferTime);

	AddNewFrameDelegate();
}

FImDbgStats::~FImDbgStats()
{
	RemoveNewFrameDelegate();
}

void FImDbgStats::ShowMenu()
{
#if STATS
	if (ImGui::Button("Stats", ImVec2(56, 22)))
	{
		if (!bIsCollecting)
		{
			StartCollectPerfData();
		}
		else
		{
			StopCollectPerfData();
		}
	}

	if (bIsCollecting)
	{
		float RenderingTime = HelperGetStat<EComplexStatField::IncAve>(FName(TEXT("STAT_TotalSceneRenderingTime")));
		UE_LOG(LogImDbg, Warning, TEXT("RenderingTime : %f]"), RenderingTime);
	}

#endif
}

void FImDbgStats::AddNewFrameDelegate()
{
#if STATS
	const FStatsThreadState& StatsState = FStatsThreadState::GetLocalState();
	OnNewFrameDelegateHandle = StatsState.NewFrameDelegate.AddRaw(this, &FImDbgStats::HandleNewFrame);
#endif
}

void FImDbgStats::RemoveNewFrameDelegate()
{
#if STATS
	const FStatsThreadState& StatsState = FStatsThreadState::GetLocalState();
	StatsState.NewFrameDelegate.Remove(OnNewFrameDelegateHandle);
#endif
}

void FImDbgStats::HandleNewFrame(int64 Frame)
{
#if 0
	FStatsThreadState& Stats = FStatsThreadState::GetLocalState();
	if (!Stats.IsFrameValid(Frame))
	{
		return;
	}

	// Stat group to request
	FName StatGroupName = FName(TEXT("STATGROUP_scenerendering"));

	// Gather stats of this group
	TArray<FName> GroupRows;
	Stats.Groups.MultiFind(StatGroupName, GroupRows);

	// 
	TSet<FName> EnabledRows;
	for (const FName& ShortName : GroupRows)
	{
		EnabledRows.Add(ShortName);
		Stats.ShortNameToLongName.FindOrAdd(ShortName);
	}

	// Filter for stats gather function
	FGroupFilter Filter(EnabledRows);

	// Empty stack node
	FRawStatStackNode HierarchyInclusive;

	// For result
	TArray<FStatMessage> NonStackStats;
	Stats.UncondenseStackStats(Frame, HierarchyInclusive, nullptr, &NonStackStats);

	for (FStatMessage Stat : NonStackStats)
	{
		FName StatName = Stat.NameAndInfo.GetRawName();

		int64 IntegerRet;
		double DoubleRet;

		switch (Stat.NameAndInfo.GetField<EStatDataType>())
		{
		case EStatDataType::ST_int64:
			IntegerRet = Stat.GetValue_int64();
			UE_LOG(LogImDbg, Warning, TEXT("Gather stat [%s : %lld]"), *StatName.ToString(), IntegerRet);
			break;
		case EStatDataType::ST_double:
			DoubleRet = Stat.GetValue_double();
			UE_LOG(LogImDbg, Warning, TEXT("Gather stat [%s : %f]"), *StatName.ToString(), DoubleRet);
			break;
		default:
			break;
		}
	}
#endif
}

void FImDbgStats::HandleNewFrameGT()
{
}

void FImDbgStats::StartCollectPerfData()
{
	if (!bIsCollecting)
	{
		bIsCollecting = true;
		StatsPrimaryEnableAdd();

		AddNewFrameDelegate();
	}
}

void FImDbgStats::StopCollectPerfData()
{
	if (bIsCollecting)
	{
		RemoveNewFrameDelegate();

		StatsPrimaryEnableSubtract();
		bIsCollecting = false;
	}
}

void FImDbgStats::GetStats()
{
#if 0
	FStatsThreadState& Stats = FStatsThreadState::GetLocalState();
	int64 LastGameFrame = Stats.GetLatestValidFrame();

	if (!Stats.IsFrameValid(LastGameFrame))
	{
		return;
	}

	// Stat group to request
	FName StatGroupName = FName(TEXT("STATGROUP_scenerendering"));

	// Gather stats of this group
	TArray<FName> GroupRows;
	Stats.Groups.MultiFind(StatGroupName, GroupRows);

	// 
	TSet<FName> EnabledRows;
	for (const FName& ShortName: GroupRows)
	{
		EnabledRows.Add(ShortName);
		Stats.ShortNameToLongName.FindOrAdd(ShortName);	
	}

	// Filter for stats gather function
	FGroupFilter Filter(EnabledRows);

	// Empty stack node
	FRawStatStackNode HierarchyInclusive;
	
	// For result
	TArray<FStatMessage> NonStackStats;

	Stats.UncondenseStackStats(LastGameFrame, HierarchyInclusive, &Filter, &NonStackStats);

	for (FStatMessage Stat: NonStackStats)
	{
		FName StatName = Stat.NameAndInfo.GetRawName();

		int64 IntegerRet;
		double DoubleRet;

		switch (Stat.NameAndInfo.GetField<EStatDataType>())
		{
		case EStatDataType::ST_int64:
			IntegerRet = Stat.GetValue_int64();
			UE_LOG(LogImDbg, Warning, TEXT("Gather stat [%s : %lld]"), *StatName.ToString(), IntegerRet);
			break;
		case EStatDataType::ST_double:
			DoubleRet = Stat.GetValue_double();
			UE_LOG(LogImDbg, Warning, TEXT("Gather stat [%s : %f]"), *StatName.ToString(), DoubleRet);
			break;
		default:
			break;
		}
	}
#endif
}

FStatsFetchThread::FStatsFetchThread(FImDbgStats& InStatsDebugger)
	:StatsDebugger(InStatsDebugger)
{
	
}

FStatsFetchThread::~FStatsFetchThread()
{
}

uint32 FStatsFetchThread::Run()
{
	return uint32();
}

void FStatsFetchThread::Stop()
{
}

void FStatsFetchThread::StartThread()
{
}

FImDbgMemoryProfiler::FImDbgMemoryProfiler(bool* bInEnabled)
{
	bEnabled = bInEnabled;
}

FImDbgMemoryProfiler::~FImDbgMemoryProfiler()
{
}

void FImDbgMemoryProfiler::ShowMenu()
{
	FPlatformMemoryStats Stats = FPlatformMemory::GetStats();



	if (ImGui::Begin("Memory Profiler", bEnabled))
	{
		if (ImGui::BeginTable("Memory Stats", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Mem");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.UsedPhysical)));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("MemPeak");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.PeakUsedPhysical)));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("MemAvailable");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.AvailablePhysical)));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("MemTotal");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.TotalPhysical)));

			ImGui::EndTable();
		}

		if (ImGui::BeginTable("Virtual Memory Stats", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("VMem");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.UsedVirtual)));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("VMemPeak");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.PeakUsedVirtual)));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("VMemAvaliable");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.AvailableVirtual)));

			ImGui::EndTable();
		}

		ImGui::End();
	}
}

FImDbgGPUProfiler::FImDbgGPUProfiler(bool* bInEnabled)
{
	bEnabled = bInEnabled;
}

FImDbgGPUProfiler::~FImDbgGPUProfiler()
{
}

// utility structure for realtime plot
struct ScrollingBuffer
{
	int MaxSize;
	int Offset;
	ImVector<ImVec2> Data;

	ScrollingBuffer(int max_size = 2000)
	{
		MaxSize = max_size;
		Offset = 0;
		Data.reserve(MaxSize);
	}

	void AddPoint(float x, float y)
	{
		if (Data.size() < MaxSize)
		{
			Data.push_back(ImVec2(x, y));
		}
		else
		{
			Data[Offset] = ImVec2(x, y);
			Offset = (Offset + 1) % MaxSize;
		}
	}

	void Erase()
	{
		if (Data.size() > 0)
		{
			Data.shrink(0);
			Offset = 0;
		}
	}
};

// utility structure for realtime plot
struct RollingBuffer
{
	float Span;
	ImVector<ImVec2> Data;

	RollingBuffer()
	{
		Span = 10.0f;
		Data.reserve(2000);
	}

	void AddPoint(float x, float y)
	{
		float xmod = fmodf(x, Span);
		if (!Data.empty() && xmod < Data.back().x)
		{
			Data.shrink(0);
		}

		Data.push_back(ImVec2(xmod, y));
	}
};

void FImDbgGPUProfiler::ShowMenu()
{
	if (ImGui::Begin("GPU Profiler", bEnabled))
	{
		// FPS/ frame time
		static RollingBuffer rdata1, rdata2;
		static float t = 0;

		static RollingBuffer GPUTimeData, RenderThreadTimeData;

		t += ImGui::GetIO().DeltaTime;

		const int FPS = static_cast<int>(1.0f / ImGui::GetIO().DeltaTime);
		const float Millis = ImGui::GetIO().DeltaTime * 1000.0f;

		rdata1.AddPoint(t, FPS);
		rdata2.AddPoint(t, Millis);
		GPUTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GGPUFrameTime));
		RenderThreadTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GRenderThreadTime));

		static float history = 5.0f;
		ImGui::SliderFloat("History", &history, 1, 5.0f, "%.1f s");
		rdata1.Span = history;
		rdata2.Span = history;
		GPUTimeData.Span = history;
		RenderThreadTimeData.Span = history;

		static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

		static float UpLimit = 120.0f;
		ImGui::SliderFloat("UpLimit", &UpLimit, 1, 200, "%.1f s");

		if (ImPlot::BeginPlot("##Rolling", ImVec2(0, 150)))
		{
			ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, 0, UpLimit);
			ImPlot::PlotLine("FPS", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
			ImPlot::PlotLine("Frame time", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
			ImPlot::PlotLine("GPU time", &GPUTimeData.Data[0].x, &GPUTimeData.Data[0].y, GPUTimeData.Data.size(), 0, 0, 2 * sizeof(float));
			ImPlot::PlotLine("RenderThread time", &RenderThreadTimeData.Data[0].x, &RenderThreadTimeData.Data[0].y, RenderThreadTimeData.Data.size(), 0, 0, 2 * sizeof(float));
			ImPlot::EndPlot();
		}

		ImGui::End();
	}
}

void FImDbgGPUProfiler::InitializeStats()
{
}

FImDbgProfiler::FImDbgProfiler()
{
}

FImDbgProfiler::~FImDbgProfiler()
{
}

void FImDbgProfiler::Initialize()
{
	GPUProfiler = MakeShared<FImDbgGPUProfiler>(&bShowGPUProfiler);
	MemoryProfiler = MakeShared<FImDbgMemoryProfiler>(&bShowMemoryProfiler);
}

void FImDbgProfiler::ShowMenu()
{
	if (ImGui::BeginMenu(IDB_PROFILER_CATRGORY))
	{
		ImGui::Checkbox("CPU Profiler", &bShowCPUProfiler);
		ImGui::Checkbox("GPU Profiler", &bShowGPUProfiler);
		ImGui::Checkbox("Memory Profiler", &bShowMemoryProfiler);
		ImGui::EndMenu();
	}

	if (bShowCPUProfiler)
	{
		ImPlot::ShowDemoWindow(&bShowCPUProfiler);
	}

	if (bShowGPUProfiler && GPUProfiler)
	{
		GPUProfiler->ShowMenu();
	}

	if (bShowMemoryProfiler && MemoryProfiler)
	{
		MemoryProfiler->ShowMenu();
	}
}

#undef LOCTEXT_NAMESPACE
