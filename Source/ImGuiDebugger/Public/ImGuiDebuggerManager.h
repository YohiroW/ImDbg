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

class FImGuiDebuggerExtension;
class FImGuiDebuggerEngine;
class FImGuiDebuggerProfiler;

UCLASS()
class UImGuiDebuggerManager: public UObject
{
    GENERATED_UCLASS_BODY()

public:
	void Initialize();
    ~UImGuiDebuggerManager();

    // dispatch console cmds
    virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;

	bool ExecuteCommand(const UObject* WorldContextObject, const FString& Command);

    void RegisterDebuggerExtension(FImGuiDebuggerExtension* InExtension);
    void UnregisterDebuggerExtension(FImGuiDebuggerExtension* InExtension);

	bool Refresh(float DeltaTime);

	FVector GetPlayerLocation();
	void ShowGPUProfiler(bool* bIsOpen);

private:
    // all registered extensions
    TArray<FImGuiDebuggerExtension*> Extensions;

	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickDelegateHandle;

	bool bRequestShowFlagUpdate;


};