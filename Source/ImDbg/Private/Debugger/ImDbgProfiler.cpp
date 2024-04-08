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
			ImGui::TableNextColumn(); ImGui::Text("Mem");
			ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.UsedPhysical)));

			ImGui::TableNextColumn(); ImGui::Text("MemPeak");
			ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.PeakUsedPhysical)));

			ImGui::TableNextColumn(); ImGui::Text("MemAvailable");
			ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.AvailablePhysical)));

			ImGui::TableNextColumn(); ImGui::Text("MemTotal");
			ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.TotalPhysical)));

			ImGui::EndTable();
		}

		if (ImGui::BeginTable("Virtual Memory Stats", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
		{
			ImGui::TableNextColumn(); ImGui::Text("VMem");
			ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.UsedVirtual)));

			ImGui::TableNextColumn(); ImGui::Text("VMemPeak");
			ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.PeakUsedVirtual)));

			ImGui::TableNextColumn(); ImGui::Text("VMemAvaliable");
			ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.AvailableVirtual)));

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
		static RollingBuffer FrameTimeData, RTTimeData, GPUTimeData, RHITTimeData;
		static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
		static float History = 10.0f;
		static float UpLimit = 33.0f;

		static float t = 0;
		t += ImGui::GetIO().DeltaTime;

		const float Millis = ImGui::GetIO().DeltaTime * 1000.0f;

		ImGui::Checkbox(GPUProfilerTypeText[EGPUProfilerType::Frame], &IsGPUProfilerEnabled[EGPUProfilerType::Frame]); ImGui::SameLine();
		ImGui::Checkbox(GPUProfilerTypeText[EGPUProfilerType::RenderThread], &IsGPUProfilerEnabled[EGPUProfilerType::RenderThread]); ImGui::SameLine();
		ImGui::Checkbox(GPUProfilerTypeText[EGPUProfilerType::GPU], &IsGPUProfilerEnabled[EGPUProfilerType::GPU]); ImGui::SameLine();
		ImGui::Checkbox(GPUProfilerTypeText[EGPUProfilerType::RHI], &IsGPUProfilerEnabled[EGPUProfilerType::RHI]);

		if (IsGPUProfilerEnabled[EGPUProfilerType::Frame])
		{
			FrameTimeData.AddPoint(t, Millis);
		}
		if (IsGPUProfilerEnabled[EGPUProfilerType::RenderThread])
		{
			RTTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GRenderThreadTime));
		}
		if (IsGPUProfilerEnabled[EGPUProfilerType::GPU])
		{
			GPUTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GGPUFrameTime));
		}
		if (IsGPUProfilerEnabled[EGPUProfilerType::RHI])
		{
			RHITTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GRHIThreadTime));
		}
		
		ImGui::SliderFloat("History", &History, 1, 10.0f, "%.1f s");
		FrameTimeData.Span = History;
		RTTimeData.Span = History;
		RHITTimeData.Span = History;
		GPUTimeData.Span = History;

		ImGui::SliderFloat("UpLimit", &UpLimit, 1, 33.0f, "%.1f s");

		if (ImPlot::BeginPlot("##Rolling", ImVec2(0, 300)))
		{
			ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, History, ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, 0, UpLimit);

			if (IsGPUProfilerEnabled[EGPUProfilerType::Frame])
			{
				ImPlot::PlotLine("Frame", &FrameTimeData.Data[0].x, &FrameTimeData.Data[0].y, FrameTimeData.Data.size(), 0, 0, 2 * sizeof(float));
			}
			if (IsGPUProfilerEnabled[EGPUProfilerType::RenderThread])
			{
				ImPlot::PlotLine("Render", &RTTimeData.Data[0].x, &RTTimeData.Data[0].y, RTTimeData.Data.size(), 0, 0, 2 * sizeof(float));
			}
			if (IsGPUProfilerEnabled[EGPUProfilerType::GPU])
			{
				ImPlot::PlotLine("GPU", &GPUTimeData.Data[0].x, &GPUTimeData.Data[0].y, GPUTimeData.Data.size(), 0, 0, 2 * sizeof(float));
			}
			if (IsGPUProfilerEnabled[EGPUProfilerType::RHI])
			{
				ImPlot::PlotLine("RHI", &RHITTimeData.Data[0].x, &RHITTimeData.Data[0].y, RHITTimeData.Data.size(), 0, 0, 2 * sizeof(float));
			}

			ImPlot::EndPlot();
		}
		ImGui::End();
	}
}

void FImDbgGPUProfiler::Initialize()
{
	for (int32 i = 0; i< EGPUProfilerType::GPUProfiler_Count; ++i)
	{
		IsGPUProfilerEnabled[i] = false;
	}
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
