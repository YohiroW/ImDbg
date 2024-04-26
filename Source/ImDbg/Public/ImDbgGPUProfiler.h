#pragma once

#include "CoreMinimal.h"
#include "Stats/StatsData.h"
#include "Stats/Stats2.h"
#include "ImDbgExtension.h"

class FImDbgGPUProfiler : public FImDbgExtension
{
public:
	enum EStatType
	{
		Frame,
		Draw,
		RHI,
		GPU,
		Num
	};

	struct GeneraStatInfo
	{
		const char* Name;
		ImDbg::FPlotScrollingData PlotData;
		bool bShowAverage = false;
	};

public:
	FImDbgGPUProfiler(bool* bInEnabled);
	~FImDbgGPUProfiler();

	virtual void ShowMenu(float InDeltaTime) override;
	void ShowGPUTimeTable();
	void ShowAverage();

	void Initialize();
	void RegisterDelegate();
	void UnRegisterDelegate();
	bool IsRegistered() const { return bIsRegistered; }
	void OnHandleNewFrame(int64 Frame);

	void SetText(const FString& InPassName, const double& InTime);

private:
	bool* bEnabled;
	bool bIsRegistered = false;
	bool bShowAverage = false;
	bool bShowBudget = false;
	FDelegateHandle OnNewFrameDelegateHandle;

	// TODO: Put into settings later
	double BudgetTime = 16.667;
	float GPUTimeThreshold = 0.4f;

	TMap<FName, double> GPUStats;
	GeneraStatInfo StatInfos[EStatType::Num];

	ImPlotAxisFlags FlagAxisX = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
	ImPlotAxisFlags FlagAxisY = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;

	// TODO: Should user engine timer for unify
	float Timer = 0.0f;
};