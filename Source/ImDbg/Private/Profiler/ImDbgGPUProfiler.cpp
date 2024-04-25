#include "ImDbgGPUProfiler.h"
#include "Stats/StatsData.h"
#include <imgui_internal.h>
#include <implot.h>

#define IMDBG_BUDGET_TAG_60 16.667
#define IMDBG_BUDGET_TAG_30 33.333

namespace ImDbg
{
	struct FGroupFilter
#if STATS
		: public IItemFilter
#endif
	{
	#if STATS
		const TSet<FName>&EnabledItems;

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
}

FImDbgGPUProfiler::FImDbgGPUProfiler(bool* bInEnabled)
{
	bEnabled = bInEnabled;
	BudgetTag60 = IMDBG_BUDGET_TAG_60;
	BudgetTag30 = IMDBG_BUDGET_TAG_30;
}

FImDbgGPUProfiler::~FImDbgGPUProfiler()
{
}

void FImDbgGPUProfiler::ShowMenu(float InDeltaTime)
{
	if (!IsRegistered())
	{
		StatsPrimaryEnableAdd();
		RegisterDelegate();
	}

	if (ImGui::Begin("GPUProfiler", bEnabled))
	{
		static ImDbg::FPlotScrollingData FrameTimeData, RTTimeData, GPUTimeData, RHITTimeData;
		static ImPlotAxisFlags FlagX = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_AutoFit;
		static ImPlotAxisFlags FlagY = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
		static ImPlotLegendFlags FlagLegend = ImPlotLegendFlags_Horizontal;

		static float t = 0;

		t += InDeltaTime;
		const float Millis = InDeltaTime * 1000.0f;

		if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
		{
			FrameTimeData.AddPoint(t, Millis);
			RTTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GRenderThreadTime));
			GPUTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GGPUFrameTime));
			RHITTimeData.AddPoint(t, FPlatformTime::ToMilliseconds(GRHIThreadTime));

			if (ImPlot::BeginPlot("GPUGraph", ImVec2(-1, 0), ImPlotFlags_NoTitle | ImPlotFlags_NoMouseText))
			{
				ImPlot::SetupAxes(nullptr, nullptr, FlagX, FlagY);
				ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);
				ImPlot::SetupAxisLimits(ImAxis_X1, t - 10.0f, t, ImGuiCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 33.3f);
				ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.4f);

				ImPlot::DragLineY(0, &BudgetTag60, ImVec4(0, 1, 0, 1), 1, ImPlotDragToolFlags_NoFit);
				ImPlot::DragLineY(1, &BudgetTag30, ImVec4(1, 0, 0, 1), 1, ImPlotDragToolFlags_NoFit);

				ImPlot::PlotLine("Frame", &FrameTimeData.Data[0].x, &FrameTimeData.Data[0].y, FrameTimeData.Data.size(), 0, FrameTimeData.Offset, 2 * sizeof(float));
				ImPlot::PlotLine("Render", &RTTimeData.Data[0].x, &RTTimeData.Data[0].y, RTTimeData.Data.size(), 0, RTTimeData.Offset, 2 * sizeof(float));
				ImPlot::PlotLine("GPU", &GPUTimeData.Data[0].x, &GPUTimeData.Data[0].y, GPUTimeData.Data.size(), 0, GPUTimeData.Offset, 2 * sizeof(float));
				ImPlot::PlotLine("RHI", &RHITTimeData.Data[0].x, &RHITTimeData.Data[0].y, RHITTimeData.Data.size(), 0, RHITTimeData.Offset, 2 * sizeof(float));

				ImPlot::PopStyleVar();
				ImPlot::EndPlot();
			}
		}
		if (ImGui::CollapsingHeader("GPU Time", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("GPU Time", ImVec2(0, 0), true);
			ImGui::DragFloat("Threshold", &GPUTimeThreshold, 0.005f, 0.0f, 10.0f, "%.3f", ImGuiSliderFlags_None);
			ImGui::Separator();
			if (ImGui::BeginTable("GPU Time", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableSetupColumn("Pass");
				ImGui::TableSetupColumn("Time(ms)", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortAscending);
				ImGui::TableSetupColumn("Budget(ms)");
				ImGui::TableHeadersRow();

				for (auto Pair: GPUStats)
				{
					if (Pair.Value > 0.0001)
					{
						FString PassName = Pair.Key.ToString();
						PassName.RemoveFromStart(TEXT("Stat_GPU_"));
						SetText(PassName, Pair.Value);
						//ImGui::TableNextColumn(); ImGui::Text("0.0");
					}
				}
				ImGui::EndTable();
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}

void FImDbgGPUProfiler::ShowGPUGeneral()
{

}

void FImDbgGPUProfiler::Initialize()
{
}

void FImDbgGPUProfiler::RegisterDelegate()
{
#if STATS
	const FStatsThreadState& StatsState = FStatsThreadState::GetLocalState();
	OnNewFrameDelegateHandle = StatsState.NewFrameDelegate.AddRaw(this, &FImDbgGPUProfiler::OnHandleNewFrame);
#endif

	bIsRegistered = true;
}

void FImDbgGPUProfiler::UnRegisterDelegate()
{
#if STATS
	const FStatsThreadState& StatsState = FStatsThreadState::GetLocalState();
	StatsState.NewFrameDelegate.Remove(OnNewFrameDelegateHandle);
#endif

	bIsRegistered = false;
}

void FImDbgGPUProfiler::OnHandleNewFrame(int64 Frame)
{
#if STATS
	// Collect GPU every 4 frame
	static int32 FrameCounter = 0;
	FrameCounter = (FrameCounter +1) % 4;
	if (FrameCounter != 0)
	{
		return;
	}

	FStatsThreadState& Stats = FStatsThreadState::GetLocalState();
	if (!Stats.IsFrameValid(Frame))
	{
		return;
	}

	// Stat group to request
	FName StatGroupName = FName(TEXT("STATGROUP_GPU"));

	// Gather stats of this group
	TArray<FName> GroupRows;
	Stats.Groups.MultiFind(StatGroupName, GroupRows);

	// 
	TSet<FName> EnabledRows;
	for (const FName& ShortName : GroupRows)
	{
		EnabledRows.Add(ShortName);
		if (FStatMessage const* LongName = Stats.ShortNameToLongName.Find(ShortName))
		{
			EnabledRows.Add(LongName->NameAndInfo.GetRawName());
		}
	}

	// Filter for stats gather function
	ImDbg::FGroupFilter Filter(EnabledRows);

	// Empty stack node
	FRawStatStackNode HierarchyInclusive;

	// For result
	TArray<FStatMessage> NonStackStats;
	Stats.UncondenseStackStats(Frame, HierarchyInclusive, &Filter, &NonStackStats);

	if (!NonStackStats.IsEmpty())
	{
		for (FStatMessage Stat : NonStackStats)
		{
			FName StatName = Stat.NameAndInfo.GetShortName();
			double CycleTime = Stat.GetValue_double();
			if (!GPUStats.Contains(StatName))
			{
				GPUStats.Add(StatName, CycleTime);
			}
			else
			{
				GPUStats[StatName] = (GPUStats[StatName] + CycleTime) * 0.5f;
			}
		}
	}
#endif
}

void FImDbgGPUProfiler::SetText(const FString& InPassName, const double& InTime)
{
	if (InTime > GPUTimeThreshold)
	{
		// Yellow
		ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.98f, 0.74f, 0.18f, 1.0f), "%s", TCHAR_TO_ANSI(*InPassName));
		ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.98f, 0.74f, 0.18f, 1.0f), "%.3lf", InTime);
	}
	else
	{
		ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*InPassName));
		ImGui::TableNextColumn(); ImGui::Text("%.3lf", InTime);
	}
}
