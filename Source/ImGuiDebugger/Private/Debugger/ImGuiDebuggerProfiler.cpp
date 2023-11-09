#include "ImGuiDebuggerProfiler.h"
#include "ImGuiDebugger.h"

FImGuiDebuggerStats::FImGuiDebuggerStats()
{
	AddNewFrameDelegate();
}

FImGuiDebuggerStats::~FImGuiDebuggerStats()
{
	const FStatsThreadState& Stats = FStatsThreadState::GetLocalState();
	Stats.NewFrameDelegate.Remove(OnNewFrameDelegateHandle);
}

void FImGuiDebuggerStats::ShowMenu()
{
#if STATS
	if (ImGui::Button("Check", ImVec2(56, 22)))
	{
		GetStats();
	}
#endif
}

void FImGuiDebuggerStats::AddNewFrameDelegate()
{
	const FStatsThreadState& Stats = FStatsThreadState::GetLocalState();
	OnNewFrameDelegateHandle = Stats.NewFrameDelegate.AddRaw(this, &FImGuiDebuggerStats::HandleNewFrame);
}

void FImGuiDebuggerStats::HandleNewFrame(int64 Frame)
{
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

	Stats.UncondenseStackStats(Frame, HierarchyInclusive, &Filter, &NonStackStats);

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
}

void FImGuiDebuggerStats::HandleNewFrameGT()
{
}

void FImGuiDebuggerStats::GetStats()
{
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