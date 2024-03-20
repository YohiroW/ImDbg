#include "ImGuiDebuggerProfiler.h"
#include "ImGuiDebugger.h"

#define LOCTEXT_NAMESPACE "ImGuiDebugger"

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

FImGuiDebuggerStats::FImGuiDebuggerStats()
{
	AddNewFrameDelegate();
}

FImGuiDebuggerStats::~FImGuiDebuggerStats()
{
	RemoveNewFrameDelegate();
}

void FImGuiDebuggerStats::ShowMenu()
{
#if STATS
	if (ImGui::Button("Check", ImVec2(56, 22)))
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
		UE_LOG(LogImGuiDebugger, Warning, TEXT("RenderingTime : %f]"), RenderingTime);
	}

#endif
}

void FImGuiDebuggerStats::AddNewFrameDelegate()
{
#if STATS
	const FStatsThreadState& Stats = FStatsThreadState::GetLocalState();
	OnNewFrameDelegateHandle = Stats.NewFrameDelegate.AddRaw(this, &FImGuiDebuggerStats::HandleNewFrame);
#endif
}

void FImGuiDebuggerStats::RemoveNewFrameDelegate()
{
#if STATS
	const FStatsThreadState& Stats = FStatsThreadState::GetLocalState();
	Stats.NewFrameDelegate.Remove(OnNewFrameDelegateHandle);
#endif
}

void FImGuiDebuggerStats::HandleNewFrame(int64 Frame)
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
			UE_LOG(LogImGuiDebugger, Warning, TEXT("Gather stat [%s : %lld]"), *StatName.ToString(), IntegerRet);
			break;
		case EStatDataType::ST_double:
			DoubleRet = Stat.GetValue_double();
			UE_LOG(LogImGuiDebugger, Warning, TEXT("Gather stat [%s : %f]"), *StatName.ToString(), DoubleRet);
			break;
		default:
			break;
		}
	}
#endif
}

void FImGuiDebuggerStats::HandleNewFrameGT()
{
}

void FImGuiDebuggerStats::StartCollectPerfData()
{
	if (!bIsCollecting)
	{
		bIsCollecting = true;
		StatsPrimaryEnableAdd();

		AddNewFrameDelegate();
	}
}

void FImGuiDebuggerStats::StopCollectPerfData()
{
	if (bIsCollecting)
	{
		RemoveNewFrameDelegate();

		StatsPrimaryEnableSubtract();
		bIsCollecting = false;
	}
}

void FImGuiDebuggerStats::GetStats()
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
			UE_LOG(LogImGuiDebugger, Warning, TEXT("Gather stat [%s : %lld]"), *StatName.ToString(), IntegerRet);
			break;
		case EStatDataType::ST_double:
			DoubleRet = Stat.GetValue_double();
			UE_LOG(LogImGuiDebugger, Warning, TEXT("Gather stat [%s : %f]"), *StatName.ToString(), DoubleRet);
			break;
		default:
			break;
		}
	}
#endif
}

FStatsFetchThread::FStatsFetchThread(FImGuiDebuggerStats& InStatsDebugger)
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

FImGuiDebuggerGPUProfiler::FImGuiDebuggerGPUProfiler()
{
	InitializeStats();
}

FImGuiDebuggerGPUProfiler::~FImGuiDebuggerGPUProfiler()
{
}

void FImGuiDebuggerGPUProfiler::ShowMenu()
{
}

void FImGuiDebuggerGPUProfiler::InitializeStats()
{
}

#undef LOCTEXT_NAMESPACE