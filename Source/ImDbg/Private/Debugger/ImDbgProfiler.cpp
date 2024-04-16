#include "ImDbgProfiler.h"
#include "ImDbgModule.h"
#include "Engine/TextureCube.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureLODSettings.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "RenderTargetPool.h"
#include "Stats/StatsData.h"
#include <imgui_internal.h>
#include <implot.h>

#define LOCTEXT_NAMESPACE "ImDbg"

#define IDB_PROFILER_CATRGORY "Profiler"
#define ToMegaByte(_BYTE_) (_BYTE_ / 1024.0f / 1024.0f )

//
// The function below is taken from AutomationBlueprintFunctionLibrary.h
// To avoid add extra dependency
//
#if STATS
template <EComplexStatField::Type ValueType, bool bCallCount = false>
double HelperGetStat(FName StatName)
{
	double Cycle = 0.0;
	if (FGameThreadStatsData* StatsData = FLatestGameThreadStatsData::Get().Latest)
	{
		for (FActiveStatGroupInfo ActiveStatGroup : StatsData->ActiveStatGroups)
		{
			for (FComplexStatMessage CountersAggregate : ActiveStatGroup.CountersAggregate)
			{
				const double IncAveValueDouble = CountersAggregate.GetValue_double(EComplexStatField::IncAve);
				const double IncMaxValueDouble = CountersAggregate.GetValue_double(EComplexStatField::IncMax);
				FName Name = CountersAggregate.GetShortName();
				if (Name == StatName)
				{
					Cycle = IncAveValueDouble;
					break;
				}
			}
		}
	}
	return Cycle;
}
#endif

FImDbgStats::FImDbgStats()
{
	//Stats.FrameTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	//Stats.FPS = round(1000.0f / Stats.FrameTime);
	//Stats.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	//Stats.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	//Stats.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
	//Stats.RHIThreadTime = FPlatformTime::ToMilliseconds(GWorkingRHIThreadTime);
	//Stats.InputLatency = FPlatformTime::ToMilliseconds(GInputLatencyTimer.DeltaTime);
	//Stats.SwapBufferTime = FPlatformTime::ToMilliseconds(GSwapBufferTime);

	AddNewFrameDelegate();
}

FImDbgStats::~FImDbgStats()
{
	StopCollectPerfData();
}

void FImDbgStats::ShowMenu()
{
	static bool bEnabled = false;
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

		bEnabled = !bEnabled;
	}

	if (bEnabled)
	{
		//float RenderingTime = HelperGetStat<EComplexStatField::IncAve>(FName(TEXT("STAT_TotalSceneRenderingTime")));
		//UE_LOG(LogImDbg, Warning, TEXT("RenderingTime : %f]"), RenderingTime);

		// case
		double Duration = HelperGetStat<EComplexStatField::ExcAve>(FName("Stat_GPU_Basepass"));
		double Duration2 = HelperGetStat<EComplexStatField::ExcAve>(FName("Stat_GPU_Total"));

		if (ImGui::Begin("GPUStats", &bIsCollecting))
		{
			ImGui::Text("%lf", Duration);
			ImGui::Text("%lf", Duration2);

			ImGui::End();
		}
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
	if (ImGui::Begin("MemoryProfiler", bEnabled))
	{
		FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
		const float PhysicalMemoryUsage = (float)Stats.UsedPhysical / (float)Stats.TotalPhysical;
		const float VirtualMemoryUsage = (float)Stats.UsedVirtual / (float)Stats.TotalVirtual;

		ImGui::Text("Physical Memory:");
		ImGui::SameLine();
		{
			const FString& ProgStr = FString::Printf(TEXT("%.02f MB / %.02f MB (%.02f%%)"), ToMegaByte(Stats.UsedPhysical), ToMegaByte(Stats.TotalPhysical), PhysicalMemoryUsage * 100.0f);
			ImGui::ProgressBar(PhysicalMemoryUsage, ImVec2(-1, 0), TCHAR_TO_ANSI(*ProgStr));
		}

		ImGui::Text("Virtual Memory:");
		ImGui::SameLine();
		{
			const FString& ProgStr = FString::Printf(TEXT("%.02f MB / %.02f MB (%.02f%%)"), ToMegaByte(Stats.UsedVirtual), ToMegaByte(Stats.TotalVirtual), VirtualMemoryUsage * 100.0f);
			ImGui::ProgressBar(VirtualMemoryUsage, ImVec2(-1, 0), TCHAR_TO_ANSI(*ProgStr));
		}

		ImGui::Separator();

		if (ImGui::BeginTabBar("MemoryDetail", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("General"))
			{
				ShowGeneralStatsView();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Texture"))
			{
				if (bRequestUpdateTextureInfo)
				{
					UpdateTextureViewInfos();
					bRequestUpdateTextureInfo = false;
				}
				ImGui::BeginChild("TabContent");
				ShowTextureMemoryView();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("UObject"))
			{
				ShowUObjectMermoryView();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("RenderTarget"))
			{
				if (bRequestUpdateRenderTargetInfo)
				{
					UpdateRenderTargetViewInfos();
					bRequestUpdateRenderTargetInfo = false;
				}
				ImGui::BeginChild("RTTabContent");
				ShowRenderTargetMemoryView();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();

			ImGui::SameLine(ImGui::GetWindowWidth() - 2 * ImGui::CalcTextSize("Update").x);
			if (ImGui::Button("Update"))
			{
				TextureViewInfoList.Empty();
				bRequestUpdateTextureInfo = true;

				RenderTargetViewInfoList.Empty();
				bRequestUpdateRenderTargetInfo = true;
			}
		}
		ImGui::End();
	}
}

void FImDbgMemoryProfiler::ShowGeneralStatsView()
{
	FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	const float PhysicalMemoryUsage = (float)Stats.UsedPhysical / (float)Stats.TotalPhysical;
	const float VirtualMemoryUsage = (float)Stats.UsedVirtual / (float)Stats.TotalVirtual;

	if (ImGui::BeginTable("MemoryStats", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
	{
		ImGui::TableNextColumn(); ImGui::Text("MemPeak");
		ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.PeakUsedPhysical)));

		ImGui::TableNextColumn(); ImGui::Text("MemAvailable");
		ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.AvailablePhysical)));

		ImGui::TableNextColumn(); ImGui::Text("VMemPeak");
		ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.PeakUsedVirtual)));

		ImGui::TableNextColumn(); ImGui::Text("VMemAvailable");
		ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*GetMemoryString(Stats.AvailableVirtual)));

		ImGui::EndTable();
	}
}

void FImDbgMemoryProfiler::ShowTextureMemoryView()
{
	const int32 TextureList_Column_Num = 8;

	// Retrieve mapping from LOD group enum value to text representation.
	static TArray<FString> TextureGroupNames = UTextureLODSettings::GetTextureGroupNames();

	if (ImGui::BeginTable("TextureView", TextureList_Column_Num, ImGuiTableFlags_RowBg 
		                                                       | ImGuiTableFlags_Borders 
		                                                       | ImGuiTableFlags_Resizable 
		                                                       | ImGuiTableFlags_Reorderable 
		                                                       | ImGuiTableFlags_SizingFixedFit
															   | ImGuiTableFlags_NoHostExtendX
															   | ImGuiTableFlags_ScrollY
	                                                           | ImGuiTableFlags_Sortable))
	{
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Dimension");
		ImGui::TableSetupColumn("InMemSize", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortAscending);
		ImGui::TableSetupColumn("Format");
		ImGui::TableSetupColumn("TextureGroup");
		ImGui::TableSetupColumn("IsStreaming");
		ImGui::TableSetupColumn("IsVirtual");
		ImGui::TableSetupColumn("UsageCount", ImGuiTableColumnFlags_PreferSortAscending);
		ImGui::TableHeadersRow();

		if (ImGuiTableSortSpecs* SortSpecs = ImGui::TableGetSortSpecs())
		{
			if (SortSpecs->SpecsDirty)
			{
				if (TextureViewInfoList.Num()> 1)
				{
					TextureViewInfoList.Sort([SortSpecs](const TSharedPtr<FTextureViewInfo> A, const TSharedPtr<FTextureViewInfo> B) -> bool
					{
						for (int32 i = 0; i< SortSpecs->SpecsCount; ++i)
						{
							const ImGuiTableColumnSortSpecs* ColumnSortSpecs = &SortSpecs->Specs[i];
							int Delta = 0;
							switch(ColumnSortSpecs->ColumnIndex)
							{
							case 0: /* Name */ 
							case 1: /* Dimension */    return A->Dimension.X > B->Dimension.X;
							case 2: /* InMemSize */    return A->InMemSize > B->InMemSize;
							case 3: /* Format */       return A->PixelFormat > B->PixelFormat;
							case 4: /* TexGroup */     return A->GroupIndex > B->GroupIndex;
							case 5: /* IsStreaming */  return A->bIsStreaming > B->bIsStreaming;
							case 6: /* IsVirtual */    return A->bIsVirtual > B->bIsVirtual;
							case 7: /* UsageCount */   return A->UsageCount > B->UsageCount;
							default: check(0); break;
							}
						}
						return A->InMemSize > B->InMemSize;
					});
					SortSpecs->SpecsDirty = false;
				}
			}
		}

		if (TextureViewInfoList.Num() > 0)
		{
			for (const TSharedPtr<FTextureViewInfo> TextureInfoPtr : TextureViewInfoList)
			{
				bool bIsValidTextureGroup = TextureGroupNames.IsValidIndex(TextureInfoPtr->GroupIndex);

				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*(TextureInfoPtr->TextureName)));
				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*FString::Printf(TEXT("%dx%d"), (int32)TextureInfoPtr->Dimension.X, (int32)TextureInfoPtr->Dimension.Y)));
				ImGui::TableNextColumn(); ImGui::Text("%.1f KB", TextureInfoPtr->InMemSize / 1024.0f);
				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(GetPixelFormatString(TextureInfoPtr->PixelFormat)));
				ImGui::TableNextColumn(); ImGui::Text("%s", bIsValidTextureGroup ? TCHAR_TO_ANSI(*TextureGroupNames[TextureInfoPtr->GroupIndex]) : "Invalid");
				ImGui::TableNextColumn(); ImGui::Text("%s", TextureInfoPtr->bIsStreaming ? "true" : "false");
				ImGui::TableNextColumn(); ImGui::Text("%s", TextureInfoPtr->bIsVirtual ? "true" : "false");
				ImGui::TableNextColumn(); ImGui::Text("%d", TextureInfoPtr->UsageCount);
			}
		}
		ImGui::EndTable();
	}
}

void FImDbgMemoryProfiler::ShowUObjectMermoryView()
{
}

void FImDbgMemoryProfiler::ShowRenderTargetMemoryView()
{
	const int32 RenderTarget_Column_Num = 6;

	if (ImGui::BeginTable("RenderTargetView", RenderTarget_Column_Num, ImGuiTableFlags_RowBg
																	 | ImGuiTableFlags_Borders
																     | ImGuiTableFlags_Resizable
																	 | ImGuiTableFlags_Reorderable
																	 | ImGuiTableFlags_SizingFixedFit
																	 | ImGuiTableFlags_NoHostExtendX
																	 | ImGuiTableFlags_ScrollY
																	 | ImGuiTableFlags_Sortable))
	{
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Dimension");
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortAscending);
		ImGui::TableSetupColumn("MipLevels");
		ImGui::TableSetupColumn("Format");
		ImGui::TableSetupColumn("Unused frames", ImGuiTableColumnFlags_PreferSortAscending);
		ImGui::TableHeadersRow();

		if (ImGuiTableSortSpecs* SortSpecs = ImGui::TableGetSortSpecs())
		{
			if (SortSpecs->SpecsDirty)
			{
				if (RenderTargetViewInfoList.Num() > 1)
				{
					RenderTargetViewInfoList.Sort([SortSpecs](const TSharedPtr<FRenderTargetViewInfo> A, const TSharedPtr<FRenderTargetViewInfo> B) -> bool
						{
							for (int32 i = 0; i < SortSpecs->SpecsCount; ++i)
							{
								const ImGuiTableColumnSortSpecs* ColumnSortSpecs = &SortSpecs->Specs[i];
								int Delta = 0;
								switch (ColumnSortSpecs->ColumnIndex)
								{
								case 0: /* Name */
								case 1: /* Dimension */      return A->Dimension.X > B->Dimension.X;
								case 2: /* Size */		     return A->Size > B->Size;
								case 3: /* MipLevels */      return A->MipLevels > B->MipLevels;
								case 4: /* Format */         return A->PixelFormat > B->PixelFormat;
								case 5: /* UnusedFrames */   return A->UnusedFrames > B->UnusedFrames;
								default: check(0); break;
								}
							}
							return A->Size > B->Size;
						});
					SortSpecs->SpecsDirty = false;
				}
			}
		}

		if (RenderTargetViewInfoList.Num() > 0)
		{
			for (const TSharedPtr<FRenderTargetViewInfo> RenderTargetInfoPtr : RenderTargetViewInfoList)
			{
				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*(RenderTargetInfoPtr->Name)));
				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(*FString::Printf(TEXT("%dx%d"), (int32)RenderTargetInfoPtr->Dimension.X, (int32)RenderTargetInfoPtr->Dimension.Y)));
				ImGui::TableNextColumn(); ImGui::Text("%.3f MB", RenderTargetInfoPtr->Size);
				ImGui::TableNextColumn(); ImGui::Text("%d", RenderTargetInfoPtr->MipLevels);
				ImGui::TableNextColumn(); ImGui::Text("%s", TCHAR_TO_ANSI(GetPixelFormatString(RenderTargetInfoPtr->PixelFormat)));
				ImGui::TableNextColumn(); ImGui::Text("%d", RenderTargetInfoPtr->UnusedFrames);
			}
		}
		ImGui::EndTable();
	}
}

void FImDbgMemoryProfiler::UpdateTextureViewInfos()
{
	// Taken from LISTTEXTURES
	// Find out how many times a texture is referenced by primitive components.
	TMap<UTexture*, int32> TextureToUsageMap;
	for (TObjectIterator<UPrimitiveComponent> It; It; ++It)
	{
		UPrimitiveComponent* PrimitiveComponent = *It;

		// Use the existing texture streaming functionality to gather referenced textures. Worth noting
		// that GetStreamingTextureInfo doesn't check whether a texture is actually streamable or not
		// and is also implemented for skeletal meshes and such.
		FStreamingTextureLevelContext LevelContext(EMaterialQualityLevel::Num, PrimitiveComponent);
		TArray<FStreamingRenderAssetPrimitiveInfo> StreamingTextures;
		PrimitiveComponent->GetStreamingRenderAssetInfo(LevelContext, StreamingTextures);

		// Increase usage count for all referenced textures
		for (int32 TextureIndex = 0; TextureIndex < StreamingTextures.Num(); TextureIndex++)
		{
			UTexture* Texture = Cast<UTexture>(StreamingTextures[TextureIndex].RenderAsset);
			if (Texture)
			{
				// Initializes UsageCount to 0 if texture is not found.
				int32 UsageCount = TextureToUsageMap.FindRef(Texture);
				TextureToUsageMap.Add(Texture, UsageCount + 1);
			}
		}
	}
	int32 NumApplicableToMinSize = 0;

	// Collect textures.
	for (TObjectIterator<UTexture> It; It; ++It)
	{
		UTexture* Texture = *It;
		UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
		UTextureCube* TextureCube = Cast<UTextureCube>(Texture);
		UVolumeTexture* Texture3D = Cast<UVolumeTexture>(Texture);
		UTextureRenderTarget2D* RenderTexture = Cast<UTextureRenderTarget2D>(Texture);

		int32				LODGroup = Texture->LODGroup;
		int32				NumMips = 1;
		int32				MaxResLODBias = Texture->GetCachedLODBias();;
		int32				MaxAllowedSizeX = FMath::Max<int32>(static_cast<int32>(Texture->GetSurfaceWidth()) >> MaxResLODBias, 1);
		int32				MaxAllowedSizeY = FMath::Max<int32>(static_cast<int32>(Texture->GetSurfaceHeight()) >> MaxResLODBias, 1);
		EPixelFormat		Format = PF_Unknown;
		int32				DroppedMips = MaxResLODBias;
		int32				CurSizeX = FMath::Max<int32>(static_cast<int32>(Texture->GetSurfaceWidth()) >> DroppedMips, 1);
		int32				CurSizeY = FMath::Max<int32>(static_cast<int32>(Texture->GetSurfaceHeight()) >> DroppedMips, 1);
		bool				bIsStreamingTexture = Texture->GetStreamingIndex() != INDEX_NONE;
		int32				MaxAllowedSize = Texture->CalcTextureMemorySizeEnum(TMC_AllMipsBiased);
		int32				CurrentSize = Texture->CalcTextureMemorySizeEnum(TMC_ResidentMips);
		int32				UsageCount = TextureToUsageMap.FindRef(Texture);
		bool				bIsForced = Texture->ShouldMipLevelsBeForcedResident() && bIsStreamingTexture;
		bool				bIsVirtual = Texture->IsCurrentlyVirtualTextured();
		bool				bIsUncompressed = Texture->IsUncompressed();

		if (Texture2D != nullptr)
		{
			NumMips = Texture2D->GetNumMips();
			MaxResLODBias = NumMips - Texture2D->GetNumMipsAllowed(false);
			MaxAllowedSizeX = FMath::Max<int32>(Texture2D->GetSizeX() >> MaxResLODBias, 1);
			MaxAllowedSizeY = FMath::Max<int32>(Texture2D->GetSizeY() >> MaxResLODBias, 1);
			Format = Texture2D->GetPixelFormat();
			DroppedMips = Texture2D->GetNumMips() - Texture2D->GetNumResidentMips();
			CurSizeX = FMath::Max<int32>(Texture2D->GetSizeX() >> DroppedMips, 1);
			CurSizeY = FMath::Max<int32>(Texture2D->GetSizeY() >> DroppedMips, 1);

			if ((NumMips >= Texture2D->GetMinTextureResidentMipCount()) && bIsStreamingTexture)
			{
				NumApplicableToMinSize++;
			}
		}
		else if (TextureCube != nullptr)
		{
			NumMips = TextureCube->GetNumMips();
			Format = TextureCube->GetPixelFormat();
		}
		else if (Texture3D != nullptr)
		{
			NumMips = Texture3D->GetNumMips();
			Format = Texture3D->GetPixelFormat();
		}
		else if (RenderTexture != nullptr)
		{
			NumMips = RenderTexture->GetNumMips();
			Format = RenderTexture->GetFormat();
		}

		TSharedPtr<FTextureViewInfo> TextureViewInfoPtr = MakeShared<FTextureViewInfo>();
		FString TexturePath = Texture->GetPathName();
	
		TextureViewInfoPtr->TextureName = FPaths::GetBaseFilename(TexturePath);
		TextureViewInfoPtr->Dimension = FVector2D(CurSizeX, CurSizeY);
		TextureViewInfoPtr->InMemSize = CurrentSize;
		TextureViewInfoPtr->PixelFormat = Format;
		TextureViewInfoPtr->GroupIndex = LODGroup;
		TextureViewInfoPtr->bIsStreaming = bIsStreamingTexture;
		TextureViewInfoPtr->bIsVirtual = bIsVirtual;
		TextureViewInfoPtr->UsageCount = TextureToUsageMap.FindRef(Texture);
		TextureViewInfoList.Add(TextureViewInfoPtr);
	}
}

void FImDbgMemoryProfiler::UpdateRenderTargetViewInfos()
{
	uint32 UnusedAllocationInKB = 0;
	for (uint32 i = 0; i < GRenderTargetPool.GetElementCount(); ++i)
	{
		if (FPooledRenderTarget* Element = GRenderTargetPool.GetElementById(i))
		{
			uint32 ElementAllocationInKB = (Element->ComputeMemorySize() + 1023) / 1024;
			if (Element->GetUnusedForNFrames() > 2)
			{
				UnusedAllocationInKB += ElementAllocationInKB;
			}

			TSharedPtr<FRenderTargetViewInfo> RenderTargetInfo = MakeShared<FRenderTargetViewInfo>();
			const FPooledRenderTargetDesc& Desc = Element->GetDesc();

			RenderTargetInfo->Name = FString(Desc.DebugName);
			RenderTargetInfo->Dimension = FVector2D(Desc.Extent.X, Desc.Extent.Y);
			RenderTargetInfo->Size = ElementAllocationInKB / 1024.0f;
			RenderTargetInfo->MipLevels = Desc.NumMips;
			RenderTargetInfo->PixelFormat = Desc.Format;
			RenderTargetInfo->UnusedFrames = Element->GetUnusedForNFrames();
			RenderTargetViewInfoList.Add(RenderTargetInfo);
		}
	}

	uint32 NumTargets = 0;
	uint32 UsedKB = 0;
	uint32 PoolKB = 0;
	GRenderTargetPool.GetStats(NumTargets, PoolKB, UsedKB);
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
		ImGui::SeparatorText("Trace");

		if (ImGui::Button("Start Trace"))
		{

		}
		ImGui::SameLine();
		if (ImGui::Button("Stop Trace"))
		{

		}
		ImGui::SameLine();
		if (ImGui::BeginMenu("Trace Channel"))
		{
			for (int32 i = 0; i < EImDbgTraceChannel::Num; i++)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				ImGui::MenuItem(ImDbgTraceChannelName[i], "", &TraceChannels[i]);
				ImGui::PopItemFlag();
			}
			ImGui::EndMenu();
		}

		ImGui::SeparatorText("Profiler");

		ImGui::Checkbox("CPU Profiler", &bShowCPUProfiler); ImGui::SameLine();
		ImGui::Checkbox("GPU Profiler", &bShowGPUProfiler); ImGui::SameLine();
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
