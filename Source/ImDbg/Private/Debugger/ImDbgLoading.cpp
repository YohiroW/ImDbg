#include "ImDbgLoading.h"
#include "EngineUtils.h"

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
			if (ImGui::RadioButton("All", FilterMode == 0)) { FilterMode = 0; } ImGui::SameLine();
			if (ImGui::RadioButton("Async", FilterMode == 1)) { FilterMode = 1; } ImGui::SameLine();
			if (ImGui::RadioButton("Sync", FilterMode == 2)) { FilterMode = 2; }
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

				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*DisplayName));
				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(StatusName));
				ImGui::TableNextColumn(); ImGui::Text("%.2f", LoadTime);
				ImGui::TableNextColumn(); ImGui::Text("%d", LevelStatus.ActorCount);
			}
		}
		ImGui::EndTable();
	}
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
