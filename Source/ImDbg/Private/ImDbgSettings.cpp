#include "ImDbgSettings.h"


UImDbgSettings::UImDbgSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TrackedShowflags.Add(FString("ShowFlag.Bounds"));
	TrackedShowflags.Add(FString("ShowFlag.StaticMeshes"));
	TrackedShowflags.Add(FString("ShowFlag.InstancedStaticMeshes"));
	TrackedShowflags.Add(FString("ShowFlag.InstancedFoliage"));
	TrackedShowflags.Add(FString("ShowFlag.HISMCOcclusionBounds"));
	TrackedShowflags.Add(FString("ShowFlag.SkeletalMeshes"));
	TrackedShowflags.Add(FString("ShowFlag.Bones"));
	TrackedShowflags.Add(FString("ShowFlag.Collision"));
	TrackedShowflags.Add(FString("ShowFlag.CollisionPawn"));
	TrackedShowflags.Add(FString("ShowFlag.CollisionVisibility"));
	TrackedShowflags.Add(FString("ShowFlag.Decals"));
	TrackedShowflags.Add(FString("ShowFlag.Diffuse"));
	TrackedShowflags.Add(FString("ShowFlag.Specular"));
	TrackedShowflags.Add(FString("ShowFlag.DynamicShadows"));
	TrackedShowflags.Add(FString("ShowFlag.ShadowFrustums"));
	TrackedShowflags.Add(FString("ShowFlag.MotionBlur"));
	TrackedShowflags.Add(FString("ShowFlag.Lighting"));
	TrackedShowflags.Add(FString("ShowFlag.Particles"));
	TrackedShowflags.Add(FString("ShowFlag.Refraction"));
	TrackedShowflags.Add(FString("ShowFlag.LODColoration"));
	TrackedShowflags.Add(FString("ShowFlag.HLODColoration"));
	TrackedShowflags.Add(FString("ShowFlag.AntiAliasing"));
	TrackedShowflags.Add(FString("ShowFlag.PostProcessing"));
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