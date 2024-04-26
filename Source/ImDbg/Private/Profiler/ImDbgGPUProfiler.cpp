#include "ImDbgGPUProfiler.h"
#include "Stats/StatsData.h"
#include <imgui_internal.h>
#include <implot.h>

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

	StatInfos[EStatType::Frame].Name = "Frame";
	StatInfos[EStatType::Draw].Name = "Draw";
	StatInfos[EStatType::RHI].Name = "RHI";
	StatInfos[EStatType::GPU].Name = "GPU";
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
		Timer += InDeltaTime;
		const float Millis = InDeltaTime * 1000.0f;

		if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
		{
			StatInfos[EStatType::Frame].PlotData.AddPoint(t, Millis);
			StatInfos[EStatType::Draw].PlotData.AddPoint(t, FPlatformTime::ToMilliseconds(GRenderThreadTime));
			StatInfos[EStatType::GPU].PlotData.AddPoint(t, FPlatformTime::ToMilliseconds(GGPUFrameTime));
			StatInfos[EStatType::RHI].PlotData.AddPoint(t, FPlatformTime::ToMilliseconds(GRHIThreadTime));

			ShowAverage();
			ImGui::Separator();

			if (ImPlot::BeginPlot("GPUGraph", ImVec2(-1, 0), ImPlotFlags_NoTitle | ImPlotFlags_NoMouseText))
			{
				ImPlot::SetupAxes(nullptr, nullptr, FlagAxisX, FlagAxisY);
				ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);
				ImPlot::SetupAxisLimits(ImAxis_X1, t - 10.0f, t, ImGuiCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 33.3f);
				ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.4f);

				for (int32 i = 0; i< EStatType::Num; ++i)
				{
					GeneraStatInfo Info = StatInfos[i];
					ImDbg::FPlotScrollingData PlotData = Info.PlotData;
					ImPlot::PlotLine(Info.Name, &PlotData.Data[0].x, &PlotData.Data[0].y, PlotData.Data.size(), 0, PlotData.Offset, 2 * sizeof(float));
				
					if (Info.bShowAverage)
					{
						double AverageTime = 0;
						for (ImVec2 Data : PlotData.Data)
						{
							AverageTime += Data.y;
						}
						AverageTime /= PlotData.Data.size();

						ImPlot::DragLineY(99 - i, &AverageTime, ImVec4(1, 1, 1, 0.75f), 1, ImPlotDragToolFlags_NoFit);
						ImPlot::TagY(AverageTime, ImVec4(1, 1, 1, 0.75f), "%s: %.1lf", Info.Name, AverageTime);
					}

					// Draw budget frame time
					ImPlot::DragLineY(0, &BudgetTime, ImVec4(0, 1, 0, 1), 1, ImPlotDragToolFlags_NoFit);
				}

				ImPlot::PopStyleVar();
				ImPlot::EndPlot();
			}
		}
		if (ImGui::CollapsingHeader("GPU Time", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ShowGPUTimeTable()
		}
		ImGui::End();
	}
}

void FImDbgGPUProfiler::ShowGPUTimeTable()
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

		for (auto Pair : GPUStats)
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

void FImDbgGPUProfiler::ShowAverage()
{
	if (ImGui::BeginMenu("Avg"))
	{
		for (int32 i = 0; i < EStatType::Num; i++)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
			ImGui::MenuItem(StatInfos[i].Name, "", &StatInfos[i].bShowAverage);
			ImGui::PopItemFlag();
		}
		ImGui::EndMenu();
	}
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
	// Collect GPU every 4 frame to avoid lag
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
