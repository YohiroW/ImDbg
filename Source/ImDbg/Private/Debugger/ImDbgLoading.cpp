#include "ImDbgLoading.h"
#include "EngineUtils.h"
#include "Engine/LevelStreamingVolume.h"
#include <implot.h>

FImDbgLoading::FImDbgLoading()
{

}

FImDbgLoading::~FImDbgLoading()
{

}

void FImDbgLoading::ShowMenu(float InDeltaTime)
{
	if (ImGui::Button("Load"))
	{
		bEnabled = !bEnabled;
	}

	if (bEnabled)
	{
		if (ImGui::Begin("LoadingViewer", &bEnabled))
		{
			ShowLoadingGraph();
			ImGui::Separator();
			ShowLoadingViewer();
			ImGui::End();
		}
	}
}

void FImDbgLoading::ShowLoadingViewer()
{
	if (ImGui::BeginTabBar("LoadingTab", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Package"))
		{
			if (ImGui::RadioButton("All", FilterMode == 0)) { FilterMode = 0; } ImGui::SameLine();
			if (ImGui::RadioButton("Async", FilterMode == 1)) { FilterMode = 1; } ImGui::SameLine();
			if (ImGui::RadioButton("Sync", FilterMode == 2)) { FilterMode = 2; }
			ImGui::Separator();
			ShowPackageLoadInfo();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Map"))
		{
			ShowMapLoadInfo();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void FImDbgLoading::ShowPackageLoadInfo()
{
	if (ImGui::BeginChild("LoadedSection", ImVec2(0, 0), true))
	{
		if (ImGui::BeginTable("LoadedTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_IsVisible);
			ImGui::TableSetupColumn("IsAsync", ImGuiTableColumnFlags_IsVisible);
			ImGui::TableSetupColumn("LoadTime", ImGuiTableColumnFlags_IsVisible);
			ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_IsVisible);
			ImGui::TableHeadersRow();

			if (!PackageLoadInfos.IsEmpty())
			{
				for (const FPackageInfo& PackInfo : PackageLoadInfos)
				{
					double LoadTime = 0.0;
					if (UPackage* Package = FindObjectFast<UPackage>(NULL, FName(PackInfo.Name)))
					{
						LoadTime = Package->GetLoadTime();
					}

					if (!IsPackageFilterOut(PackInfo))
					{
						ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*FPackageName::GetShortName(PackInfo.Name)));
						ImGui::TableNextColumn(); ImGui::Text("%s", PackInfo.bIsAsync ? "true" : "false");
						ImGui::TableNextColumn(); ImGui::Text("%.3f", LoadTime);
						ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*PackInfo.Name));
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
}

void FImDbgLoading::ShowMapLoadInfo()
{
	if (ImGui::BeginTable("LoadedPackages", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
	{
		const TArray<FSubLevelStatus> SubLevelsStatusList = GetSubLevelsStatus(GWorld);

		ImGui::TableSetupColumn("Map", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("LoadTime", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("Actors", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableHeadersRow();

		if (SubLevelsStatusList.Num())
		{
			for (int32 LevelIdx = 0; LevelIdx < SubLevelsStatusList.Num(); ++LevelIdx)
			{
				const FSubLevelStatus& LevelStatus = SubLevelsStatusList[LevelIdx];
				FString DisplayName = FPackageName::GetShortName(LevelStatus.PackageName.ToString());
				const TCHAR* StatusName = ULevelStreaming::GetLevelStreamingStatusDisplayName(LevelStatus.StreamingStatus);
				float LoadTime = 0.0f;

				UPackage* LevelPackage = FindObjectFast<UPackage>(NULL, LevelStatus.PackageName);
				if (LevelPackage
					&& (LevelPackage->GetLoadTime() > 0)
					&& (LevelStatus.StreamingStatus != LEVEL_Unloaded))
				{
					LoadTime = LevelPackage->GetLoadTime();
				}

				if (LevelStatus.bPlayerInside)
				{
					DisplayName = FString("> ").Append(DisplayName);
				}

				ImVec4 Color = GetLevelStatusColor(LevelStatus.StreamingStatus);

				ImGui::TableNextColumn(); ImGui::TextColored(Color, "%s", TCHAR_TO_ANSI(*DisplayName));
				ImGui::TableNextColumn(); ImGui::TextColored(Color, "%s", TCHAR_TO_ANSI(StatusName));
				ImGui::TableNextColumn(); ImGui::TextColored(Color, "%.2f", LoadTime);
				ImGui::TableNextColumn(); ImGui::TextColored(Color, "%d", LevelStatus.ActorCount);
			}
		}
		ImGui::EndTable();
	}
}

void FImDbgLoading::ShowLoadingGraph()
{
	if (bUpdateLevelInfo)
	{
		UpdateLevelInfo();
		bUpdateLevelInfo = false;
	}
	 
	if (ImPlot::BeginPlot("MapGraph", ImVec2(-1, 0), ImPlotFlags_NoTitle))
	{
		ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
		ImPlot::SetupAxisLimits(ImAxis_X1, -15000.0f, 15000.0f);
		ImPlot::SetupAxisLimits(ImAxis_Y1, -15000.0f, 15000.0f);

		FVector Location = FImDbgUtil::GetPlayerLocation();
		ImVec2 PlotLocation = ImPlot::PlotToPixels(ImPlotPoint(Location.X, Location.Y));

		ImPlot::PushPlotClipRect();

		// Draw player
		ImPlot::GetPlotDrawList()->AddCircleFilled(PlotLocation, 8, IM_COL32(0, 255, 0, 255), 4);

		for (ALevelStreamingVolume* Volume : LevelStreamingVolumes)
		{
			FBox Box = GetVolumeExtent(Volume);
			DrawRect(Box.Min, Box.Max);
		}

		ImPlot::PopPlotClipRect();
		ImPlot::EndPlot();
	}
}

void FImDbgLoading::DrawRect(const FVector& InMin, const FVector& InMax)
{
	ImVec2 PlotMin = ImPlot::PlotToPixels(ImPlotPoint(InMin.X, InMin.Y));
	ImVec2 PlotMax = ImPlot::PlotToPixels(ImPlotPoint(InMax.X, InMax.Y));
	ImPlot::GetPlotDrawList()->AddRect(PlotMin, PlotMax, IM_COL32(250, 188, 46, 255));
}

void FImDbgLoading::Initialize()
{
	LoadedAsyncDelegateHandle = FCoreDelegates::OnAsyncLoadPackage.AddRaw(this, &FImDbgLoading::OnPackageLoadedAsync);
	LoadedSyncDelegateHandle = FCoreDelegates::OnSyncLoadPackage.AddRaw(this, &FImDbgLoading::OnPackageLoadedSync);
}

bool FImDbgLoading::IsPackageFilterOut(const FPackageInfo& InPackInfo)
{
	if (FilterMode == 0)
	{
		return false;
	}

	return FilterMode == 2? InPackInfo.bIsAsync: !InPackInfo.bIsAsync;
}

void FImDbgLoading::OnPackageLoadedAsync(const FString& InPackageName)
{
	PackageLoadInfos.Add({InPackageName, true });
}

void FImDbgLoading::OnPackageLoadedSync(const FString& InPackageName)
{
	PackageLoadInfos.Add({ InPackageName, false });
}

void FImDbgLoading::UpdateLevelInfo()
{
	LevelStreamingVolumes = GetLevelStreamingVolumes();
}

ImVec4 FImDbgLoading::GetLevelStatusColor(const EStreamingStatus& InStreamingStatus) const
{
	switch (InStreamingStatus)
	{
	case LEVEL_Unloaded:				return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	case LEVEL_UnloadedButStillAround:  return ImVec4(0.66f, 0.02f, 0.89f, 1.0f);
	case LEVEL_Loading:				    return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	case LEVEL_Loaded:					return ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
	case LEVEL_MakingVisible:			return ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	case LEVEL_Visible:					return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	case LEVEL_Preloading:				return ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	case LEVEL_FailedToLoad:			return ImVec4(0.56f, 0.14f, 0.42f, 1.0f);
	case LEVEL_MakingInvisible:			return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
	default:							return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	};
}

TArray<ALevelStreamingVolume*> FImDbgLoading::GetLevelStreamingVolumes()
{
	TArray<ALevelStreamingVolume*> Volumes;
	for (TObjectIterator<ALevelStreamingVolume> Iter; Iter; ++Iter)
	{
		ALevelStreamingVolume* Volume = *Iter;
		if (IsValid(Volume))
		{
			Volumes.Add(Volume);
		}
	}

	return Volumes;
}

FBox FImDbgLoading::GetVolumeExtent(ALevelStreamingVolume* InVolume)
{
	check(InVolume);

	FBox Box;
	FBoxSphereBounds Bounds = InVolume->GetBounds();
	Box.Min = Bounds.Origin - Bounds.BoxExtent.X / 2;
	Box.Max = Bounds.Origin + Bounds.BoxExtent.Y / 2;

	return Box;
}
