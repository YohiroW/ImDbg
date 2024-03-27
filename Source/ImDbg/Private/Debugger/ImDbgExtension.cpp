#include "ImDbgExtension.h"
#include "ImDbg.h"

#include "Kismet/GameplayStatics.h"

bool FImDbgEntry::operator== (const FImDbgEntry& Other)
{
	return this->Section == Other.Section
		&& this->Command == Other.Command
		&& this->Args == Other.Args
		&& this->DisplayName == Other.DisplayName;
}

void FImDbgEntry::Execure()
{
	FString Argument = bToggled ? FString("1") : FString("0");
	FString Cmd = FString::Printf(TEXT("%s %s"), *Command, *Argument);

	UKismetSystemLibrary::ExecuteConsoleCommand(GEngine->GetWorld(), Cmd);
}

FImDbgExtension::FImDbgExtension()
{
}

FImDbgExtension::~FImDbgExtension()
{
	Release();
}

void FImDbgExtension::RegisterDebuggerEntry(const FImDbgEntry& Entry)
{
	Entries.Add(Entry);
}

void FImDbgExtension::UnregisterDebuggerEntry(const FImDbgEntry& Entry)
{
	Entries.Remove(Entry);
}

void FImDbgExtension::Release()
{
	if (Entries.Num() == 0)
	{
		return;
	}

	Entries.Empty();
}

void FImDbgExtension::ShowMenu()
{
	UE_LOG(LogImDbg, Log, TEXT("To be implemented."));
}

void FImDbgExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, int32& OutValue)
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

void FImDbgExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, float& OutValue)
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

void FImDbgExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, int32& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;

	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atoi(*ValueString);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [name]:%s [value]:%d"), *InCVarString, *OutCommand, OutValue);
}

void FImDbgExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, float& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;

	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atof(*ValueString);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [name]:%s [value]:%f"), *InCVarString, *OutCommand, OutValue);
}

void FImDbgExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCategory, FString& OutCommandDisplayName)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	InCVarString.Split(TEXT("."), &OutCategory, &OutCommandDisplayName);

	UE_LOG(LogImDbg, Log, TEXT("Parsing[%s]: [category]:%s [display]:%s"), *InCVarString, *OutCategory, *OutCommandDisplayName);
}

TMap<FString, IConsoleVariable*> FImDbgExtension::GetCVarList(const FString& InCategory)
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
