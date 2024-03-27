#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "Engine/EngineTypes.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Interface.h"
#include "Containers/Ticker.h"
#include "ImDbgExtension.h"

#include "ImDbgManager.generated.h"

#define IDG_WHITELIST TEXT("whitelist")

class FImDbgExtension;
class FImDbgEngine;
class FImDbgProfiler;

UCLASS()
class UImDbgManager: public UObject
{
    GENERATED_UCLASS_BODY()

public:
	~UImDbgManager();
	void Initialize();
	void InitializeImGuiStyle();

	bool ExecuteCommand(const UObject* WorldContextObject, const FString& Command);

    void RegisterDebuggerExtension(FImDbgExtension* InExtension);
    void UnregisterDebuggerExtension(FImDbgExtension* InExtension);

	bool Refresh(float DeltaTime);

	void ShowMainMenu(float DeltaTime);
	void ShowOverlay();

	FVector GetPlayerLocation();
	void ShowGPUProfiler(bool* bIsOpen);

	void LoadWhitelist(const FString& Whitelist = IDG_WHITELIST);
	bool IsTracked(const FString& InCommand);

	TArray<FString> GetCommandsByCategory(const FString& InCategory);

private:
	bool bIsImGuiInitialized = false;
	bool bIsDebuggerInitialized = false;

    // All registered extensions
    TArray<FImDbgExtension*> Extensions;

	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickDelegateHandle;

	TArray<FString> TrackedCommands;
};