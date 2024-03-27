#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"

#include "ImGuiDebuggerSettings.generated.h"

UCLASS(config=Engine, defaultconfig, meta=(DisplayName="ImGuiDebugger"))
class UImGuiDebuggerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UImGuiDebuggerSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Key to toggle debugger overlay
	UPROPERTY(config, EditAnywhere, Category = "Input")
	FKey OverlayToggleKey;

	// Section for QA, can be jira or issue page
	UPROPERTY(Config, EditAnywhere, Category = "QA")
	FString IssuePage = FString("blog.yohiro.cn");

#if WITH_EDITOR
	// Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;

	virtual FText GetSectionText() const override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// END UDeveloperSettings Interface

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnImGuiDebuggerSettingsChanged, const FName&, const UImGuiDebuggerSettings*);
	
	static FOnImGuiDebuggerSettingsChanged& OnSettingsChanged();

	static FOnImGuiDebuggerSettingsChanged SettingsChangedDelegate;
#endif
};
