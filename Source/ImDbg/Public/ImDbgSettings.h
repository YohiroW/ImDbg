#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"

#include "ImDbgSettings.generated.h"

UCLASS(config=Engine, defaultconfig, meta=(DisplayName="ImDbg"))
class UImDbgSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UImDbgSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnImDbgSettingsChanged, const FName&, const UImDbgSettings*);
	
	static FOnImDbgSettingsChanged& OnSettingsChanged();

	static FOnImDbgSettingsChanged SettingsChangedDelegate;
#endif
};
