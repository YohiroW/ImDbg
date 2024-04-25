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

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

#define IDG_WHITELIST TEXT("whitelist")

#if WITH_EDITOR
extern UNREALED_API UEditorEngine* GEditor;
#endif

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

    void RegisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension);
    void UnregisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension);

	void OnViewportResized(FViewport* Viewport, uint32 Unused);

	void ShowMainMenu(float InDeltaTime);
	
	// Show stats like 'stat unit'
	void ShowOverlay();

	bool IsTracked(const FString& InCommand);
	TArray<FString> GetCommandsByCategory(const FString& InCategory);

	// TODO: try use FStatUnitData to gather stats data
	void UpdateStats();

	// ImGui section
	ImGuiIO& GetImGuiIO() const;

private:
	bool bIsImGuiInitialized = false;

    // All registered extensions
    TArray<TSharedPtr<FImDbgExtension>> Extensions;

	TArray<FString> TrackedCommands;

	/** Time that has transpired since the last draw call */
	double LastTime;

	float FrameTime = 0.0f;
	float GameThreadTime = 0.0f;
	float RenderThreadTime = 0.0f;
	float RHITTime = 0.0f;
	float InputLatencyTime = 0.0f;
	float GPUFrameTime = 0.0f;
	float SwapBufferTime = 0.0f;

	FDelegateHandle OnViewportResizedHandle;
};