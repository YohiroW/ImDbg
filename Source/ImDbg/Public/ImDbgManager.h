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

#define IDG_WHITELIST TEXT("whitelist")

class FImDbgExtension;
class FImDbgEngine;
class FImDbgProfiler;

class FImDbgManager: public FTickableGameObject
{
public:
	FImDbgManager();
	~FImDbgManager();

	void Initialize();
	void InitializeImGuiStyle();

	// FTickableGameObject implementation Begin
	virtual UWorld* GetTickableGameObjectWorld() const override { return GWorld; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsAllowedToTick() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject implementation End

	bool ExecuteCommand(const UObject* WorldContextObject, const FString& Command);

    void RegisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension);
    void UnregisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension);

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
	bool bIsManagerInitialized = false;

    // All registered extensions
    TArray<TSharedPtr<FImDbgExtension>> Extensions;

	TArray<FString> TrackedCommands;
};