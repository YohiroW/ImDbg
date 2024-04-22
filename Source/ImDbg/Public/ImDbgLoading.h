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

	void Initialize();
	bool IsPackageFilterOut(const FPackageInfo& InPackInfo);

	void OnPackageLoadedAsync(const FString& InPackageName);
	void OnPackageLoadedSync(const FString& InPackageName);

private:
	bool bEnabled = false;

	// 0 - all, 1 - async only, 2 - sync only
	int8 FilterMode = 0; 

	FDelegateHandle LoadedAsyncDelegateHandle;
	FDelegateHandle LoadedSyncDelegateHandle;

	TArray<FPackageInfo> PackageLoadInfos;
};
