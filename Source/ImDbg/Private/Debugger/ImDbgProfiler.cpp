#include "ImDbgProfiler.h"
#include "ImDbgModule.h"
#include "Engine/TextureCube.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureLODSettings.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "Stats/StatsData.h"
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

		if (ImGui::Button("Update"))
		{
			TextureViewInfoList.Empty();
			bRequestUpdateTextureInfo = true;
		}

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

				ShowTextureMemoryView();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("UObject"))
			{
				ShowUObjectMermoryView();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("RenderTarget"))
			{
				ShowRenderTargetMemoryView();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
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
	static float ColumnWidths[TextureList_Column_Num];

	// Retrieve mapping from LOD group enum value to text representation.
	static TArray<FString> TextureGroupNames = UTextureLODSettings::GetTextureGroupNames();

	if (ImGui::BeginTable("TextureView", TextureList_Column_Num, ImGuiTableFlags_RowBg 
		                                                               | ImGuiTableFlags_Borders 
		                                                               | ImGuiTableFlags_Resizable 
		                                                               | ImGuiTableFlags_Reorderable 
		                                                               | ImGuiTableFlags_SizingFixedFit
																	   | ImGuiTableFlags_NoHostExtendX
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
	if (ImGui::Begin("GPUProfiler", bEnabled))
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

		ImGui::SliderFloat("UpLimit", &UpLimit, 0, 33.0f, "%.1f s");

		if (ImPlot::BeginPlot("GPUGeneral"))
		{
			ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
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
