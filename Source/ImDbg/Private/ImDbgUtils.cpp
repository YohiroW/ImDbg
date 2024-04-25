#include "ImDbgUtils.h"

TMap<FString, IConsoleVariable*> FImDbgUtil::GetCVarList(const FString& InCategory)
{
	TMap<FString, IConsoleVariable*> CVarMap;

	FString Cmd = InCategory;
	Cmd.TrimStartAndEndInline();

	IConsoleManager::Get().ForEachConsoleObjectThatStartsWith(FConsoleObjectVisitor::CreateLambda(
		[&](const TCHAR* Key, IConsoleObject* ConsoleObject)
		{
			if (IConsoleVariable* AsVariable = ConsoleObject->AsVariable())
			{
				CVarMap.Add(FString(Key), AsVariable);
			}
		}),
		*InCategory);

	return CVarMap;
}


void FImDbgUtil::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, int32& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;
	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atoi(*ValueString);

	FString Category;
	InCVarString.Split(TEXT("."), &Category, &OutCommandName);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [category]:%s [name]:%s [value]:%d"), *OutCommand, *Category, *OutCommandName, OutValue);
}

void FImDbgUtil::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, float& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;
	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atof(*ValueString);

	FString Category;
	InCVarString.Split(TEXT("."), &Category, &OutCommandName);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [category]:%s [name]:%s [value]:%f"), *OutCommand, *Category, *OutCommandName, OutValue);
}

void FImDbgUtil::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, int32& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;

	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atoi(*ValueString);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [name]:%s [value]:%d"), *InCVarString, *OutCommand, OutValue);
}

void FImDbgUtil::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, float& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;

	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atof(*ValueString);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [name]:%s [value]:%f"), *InCVarString, *OutCommand, OutValue);
}

void FImDbgUtil::ParseConsoleVariable(FString& InCVarString, FString& OutCategory, FString& OutCommandDisplayName)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	InCVarString.Split(TEXT("."), &OutCategory, &OutCommandDisplayName);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [category]:%s [display]:%s"), *InCVarString, *OutCategory, *OutCommandDisplayName);
}

FVector FImDbgUtil::GetPlayerLocation()
{
	ULocalPlayer* Player = (GEngine && GWorld) ? GEngine->GetFirstGamePlayer(GWorld) : nullptr;
	FVector PlayerLoc = FVector::ZeroVector;
	if (Player)
	{
		APlayerController* Controller = Player->GetPlayerController(GWorld);

		if (Controller)
		{
			if (auto Pawn = Controller->GetPawn())
			{
				PlayerLoc = Pawn->K2_GetActorLocation();
			}
		}
	}

	return PlayerLoc;
}
