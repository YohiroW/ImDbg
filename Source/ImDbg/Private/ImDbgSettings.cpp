#include "ImDbgSettings.h"


UImDbgSettings::UImDbgSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
UImDbgSettings::FOnImDbgSettingsChanged UImDbgSettings::SettingsChangedDelegate;

FName UImDbgSettings::GetCategoryName() const
{
	return FName("Plugins");
}

FText UImDbgSettings::GetSectionText() const
{
	return NSLOCTEXT("ImDbgPlugin", "ImDbgSextion", "ImDbg");
}

void UImDbgSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property)
	{
		SettingsChangedDelegate.Broadcast(PropertyChangedEvent.Property->GetFName(), this);
	}
}

UImDbgSettings::FOnImDbgSettingsChanged& UImDbgSettings::OnSettingsChanged()
{
	return SettingsChangedDelegate;
}
#endif