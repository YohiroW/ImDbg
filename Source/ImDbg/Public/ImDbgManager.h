#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "Engine/EngineTypes.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Interface.h"
#include "Containers/Ticker.h"
#include "ImGuiDebuggerExtension.h"

#include "ImGuiDebuggerManager.generated.h"

#define IDG_WHITELIST TEXT("whitelist")

class FImGuiDebuggerExtension;
class FImGuiDebuggerEngine;
class FImGuiDebuggerProfiler;

UCLASS()
class UImGuiDebuggerManager: public UObject
{
    GENERATED_UCLASS_BODY()

public:
	~UImGuiDebuggerManager();
	void Initialize();
	void InitializeImGuiStyle();

	bool ExecuteCommand(const UObject* WorldContextObject, const FString& Command);

    void RegisterDebuggerExtension(FImGuiDebuggerExtension* InExtension);
    void UnregisterDebuggerExtension(FImGuiDebuggerExtension* InExtension);

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
    TArray<FImGuiDebuggerExtension*> Extensions;

	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickDelegateHandle;

	TArray<FString> TrackedCommands;
};