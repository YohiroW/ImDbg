#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImDbgExtension.h"

class FImDbgGPUProfiler : public FImDbgExtension
{
public:
	FImDbgGPUProfiler(bool* bInEnabled);
	~FImDbgGPUProfiler();

	virtual void ShowMenu(float InDeltaTime) override;
	void ShowGPUGeneral();

	void Initialize();
	void RegisterDelegate();
	void UnRegisterDelegate();
	bool IsRegistered() const { return bIsRegistered; }
	void OnHandleNewFrame(int64 Frame);

	void SetText(const FString& InPassName, const double& InTime);

private:
	//FImDbgStats Stats;
	bool* bEnabled;
	bool bIsRegistered = false;
	FDelegateHandle OnNewFrameDelegateHandle;

	double BudgetTag60;
	double BudgetTag30;
	float GPUTimeThreshold = 0.4f;

	TMap<FName, double> GPUStats;
};