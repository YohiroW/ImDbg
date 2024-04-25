#pragma once

#include "CoreMinimal.h"
#include "ImDbgExtension.h"

class FImDbgLoading : public FImDbgExtension
{
public:
	struct FPackageInfo
	{
		FString Name;
		bool bIsAsync;
	};

public:
	FImDbgLoading();
	~FImDbgLoading();

	virtual void ShowMenu(float InDeltaTime) override;
	void ShowLoadingViewer();
	void ShowPackageLoadInfo();
	void ShowMapLoadInfo();
	void ShowLoadingGraph();

	// InMin = upper-left, InMax = lower-right
	void DrawRect(const FVector& InMin, const FVector& InMax);

	void Initialize();
	bool IsPackageFilterOut(const FPackageInfo& InPackInfo);

	void OnPackageLoadedAsync(const FString& InPackageName);
	void OnPackageLoadedSync(const FString& InPackageName);

	void UpdateLevelInfo();

	ImVec4 GetLevelStatusColor(const EStreamingStatus& InStreamingStatus) const;

	// For traditional level streaming
	TArray<ALevelStreamingVolume*> GetLevelStreamingVolumes();
	FBox GetVolumeExtent(ALevelStreamingVolume* InVolume);

	// For world partition


private:
	bool bEnabled = false;
	bool bUpdateLevelInfo = true;

	// 0 - all, 1 - async only, 2 - sync only
	int8 FilterMode = 0; 

	FDelegateHandle LoadedAsyncDelegateHandle;
	FDelegateHandle LoadedSyncDelegateHandle;

	TArray<FPackageInfo> PackageLoadInfos;

	TArray<ALevelStreamingVolume*> LevelStreamingVolumes;
};
