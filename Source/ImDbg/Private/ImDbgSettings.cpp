#include "ImGuiDebuggerSettings.h"


UImGuiDebuggerSettings::UImGuiDebuggerSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
UImGuiDebuggerSettings::FOnImGuiDebuggerSettingsChanged UImGuiDebuggerSettings::SettingsChangedDelegate;

FName UImGuiDebuggerSettings::GetCategoryName() const
{
	return FName("Plugins");
}

FText UImGuiDebuggerSettings::GetSectionText() const
{
	return NSLOCTEXT("ImGuiDebuggerPlugin", "ImGuiDebuggerSextion", "ImGuiDebugger");
}

void UImGuiDebuggerSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property)
	{
		SettingsChangedDelegate.Broadcast(PropertyChangedEvent.Property->GetFName(), this);
	}
}

UImGuiDebuggerSettings::FOnImGuiDebuggerSettingsChanged& UImGuiDebuggerSettings::OnSettingsChanged()
{
	return SettingsChangedDelegate;
}
#endif