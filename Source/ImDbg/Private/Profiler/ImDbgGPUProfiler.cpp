#include "ImDbgGPUProfiler.h"
#include "Stats/StatsData.h"
#include <imgui_internal.h>
#include <implot.h>

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
	if (!IsRegistered())
	{
		StatsPrimaryEnableAdd();
		RegisterDelegate();
	}

	if (ImGui::Begin("GPUProfiler", bEnabled))
	{
		static RollingBuffer FrameTimeData, RTTimeData, GPUTimeData, RHITTimeData;
		static ImPlotAxisFlags FlagX = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_AutoFit;
		static ImPlotAxisFlags FlagY = ImPlotAxisFlags_AutoFit;
		static ImPlotLegendFlags FlagLegend = ImPlotLegendFlags_Horizontal;
		static float History = 10.0f;
		static float UpLimit = 33.0f;

		static float t = 0;
		t += ImGui::GetIO().DeltaTime;
		const float Millis = ImGui::GetIO().DeltaTime * 1000.0f;

		if (ImGui::CollapsingHeader("General"))
		{
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

			FrameTimeData.Span = History;
			RTTimeData.Span = History;
			RHITTimeData.Span = History;
			GPUTimeData.Span = History;

			if (ImPlot::BeginPlot(""))
			{
				ImPlot::SetupAxes(nullptr, nullptr, FlagX, FlagY);
				ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_Horizontal);
				ImPlot::SetupAxisLimits(ImAxis_X1, 0, History, ImGuiCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, UpLimit);
				ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);

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

				ImPlot::PopStyleVar();
				ImPlot::EndPlot();
			}

			if (ImGui::BeginMenu("Threads"))
			{
				for (int32 i = 0; i < EGPUProfilerType::GPUProfiler_Count; i++)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
					ImGui::MenuItem(GPUProfilerTypeText[i], "", &IsGPUProfilerEnabled[i]);
					ImGui::PopItemFlag();
				}
				ImGui::EndMenu();
			}
		}
		if (ImGui::CollapsingHeader("GPU Time"))
		{
			ImGui::BeginChild("GPU Time", ImVec2(0, 0), true);
			if (ImGui::BeginTable("GPU Time", 2, ImGuiTableFlags_RowBg
				| ImGuiTableFlags_Borders
				| ImGuiTableFlags_SizingFixedFit))
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

						ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*PassName));
						ImGui::TableNextColumn(); ImGui::Text("%.3lf", Pair.Value);
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
	for (int32 i = 0; i< EGPUProfilerType::GPUProfiler_Count; ++i)
	{
		IsGPUProfilerEnabled[i] = false;
	}
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
	FGroupFilter Filter(EnabledRows);

	// Empty stack node
	FRawStatStackNode HierarchyInclusive;

	// For result
	TArray<FStatMessage> NonStackStats;
	Stats.UncondenseStackStats(Frame, HierarchyInclusive, &Filter, &NonStackStats);

	if (NonStackStats.Num() > 0)
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
}