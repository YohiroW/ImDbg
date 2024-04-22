#include "ImDbgMemoryProfiler.h"
#include "Engine/TextureCube.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureLODSettings.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "RenderTargetPool.h"

#define ToMegaByte(_BYTE_) (_BYTE_ / 1024.0f / 1024.0f )

FImDbgMemoryProfiler::FImDbgMemoryProfiler(bool* bInEnabled)
{
	bEnabled = bInEnabled;
}

FImDbgMemoryProfiler::~FImDbgMemoryProfiler()
{
}

void FImDbgMemoryProfiler::ShowMenu(float InDeltaTime)
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
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("Dimension", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("InMemSize", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("Format", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("TextureGroup", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("IsStreaming", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("IsVirtual", ImGuiTableColumnFlags_IsVisible);
		ImGui::TableSetupColumn("UsageCount", ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_IsVisible);
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

		if (!TextureViewInfoList.IsEmpty())
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

		if (!RenderTargetViewInfoList.IsEmpty())
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