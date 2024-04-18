#pragma once

#include "CoreMinimal.h"
#include "ImDbgExtension.h"

class FImDbgMemoryProfiler : public FImDbgExtension
{
public:
	struct FTextureViewInfo
	{
		FString TextureName;
		FVector2D Dimension;
		float InMemSize;
		EPixelFormat PixelFormat;
		int32 GroupIndex;
		bool bIsStreaming;
		bool bIsVirtual;
		int32 UsageCount;
	};

	struct FRenderTargetViewInfo
	{
		FString Name;
		FVector2D Dimension;
		float Size;
		int32 MipLevels;
		EPixelFormat PixelFormat;
		int32 UnusedFrames;
	};

public:
	FImDbgMemoryProfiler(bool* bInEnabled);
	~FImDbgMemoryProfiler();

	virtual void ShowMenu() override;
	void ShowGeneralStatsView();
	void ShowTextureMemoryView();
	void ShowUObjectMermoryView();
	void ShowRenderTargetMemoryView();

	void UpdateTextureViewInfos();
	void UpdateRenderTargetViewInfos();

private:
	bool* bEnabled;

	bool bRequestUpdateTextureInfo = true;
	TArray<TSharedPtr<FTextureViewInfo>> TextureViewInfoList;

	bool bRequestUpdateRenderTargetInfo = true;
	TArray<TSharedPtr<FRenderTargetViewInfo>> RenderTargetViewInfoList;
};