#include "ImGuiDebuggerExtension.h"
#include "ImGuiDebugger.h"

#include "Kismet/GameplayStatics.h"

bool FImGuiDebugEntry::operator== (const FImGuiDebugEntry& Other)
{
	return this->Section == Other.Section
		&& this->Command == Other.Command
		&& this->Args == Other.Args
		&& this->DisplayName == Other.DisplayName;
}

void FImGuiDebugEntry::Execure()
{
	FString Argument = bToggled ? FString("1") : FString("0");
	FString Cmd = FString::Printf(TEXT("%s %s"), *Command, *Argument);

	UKismetSystemLibrary::ExecuteConsoleCommand(GEngine->GetWorld(), Cmd);
}

FImGuiDebuggerExtension::FImGuiDebuggerExtension()
{
}

FImGuiDebuggerExtension::~FImGuiDebuggerExtension()
{
	Release();
}

void FImGuiDebuggerExtension::RegisterDebuggerEntry(const FImGuiDebugEntry& Entry)
{
	Entries.Add(Entry);
}

void FImGuiDebuggerExtension::UnregisterDebuggerEntry(const FImGuiDebugEntry& Entry)
{
	Entries.Remove(Entry);
}

void FImGuiDebuggerExtension::Release()
{
	if (Entries.Num() == 0)
	{
		return;
	}

	Entries.Empty();
}

void FImGuiDebuggerExtension::ShowMenu()
{
	UE_LOG(LogImGuiDebugger, Log, TEXT("To be implemented."));
}

void FImGuiDebuggerExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, int32& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;
	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atoi(*ValueString);

	FString Category;
	InCVarString.Split(TEXT("."), &Category, &OutCommandName);

	UE_LOG(LogImGuiDebugger, Log, TEXT("Parsing[%s]: [category]:%s [name]:%s [value]:%d"), *OutCommand, *Category, *OutCommandName, OutValue);
}

void FImGuiDebuggerExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, float& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;
	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atof(*ValueString);

	FString Category;
	InCVarString.Split(TEXT("."), &Category, &OutCommandName);

	UE_LOG(LogImGuiDebugger, Log, TEXT("Parsing[%s]: [category]:%s [name]:%s [value]:%f"), *OutCommand, *Category, *OutCommandName, OutValue);
}

void FImGuiDebuggerExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, int32& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;

	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atoi(*ValueString);

	UE_LOG(LogImGuiDebugger, Log, TEXT("Parsing[%s]: [name]:%s [value]:%d"), *InCVarString, *OutCommand, OutValue);
}

void FImGuiDebuggerExtension::ParseConsoleVariable(FString& InCVarString, FString& OutCommand, float& OutValue)
{
	InCVarString.TrimStartInline();
	InCVarString.TrimEndInline();

	FString ValueString;

	InCVarString.Split(TEXT(" "), &OutCommand, &ValueString);
	OutValue = FCString::Atof(*ValueString);

	UE_LOG(LogImGuiDebugger, Log, TEXT("Parsing[%s]: [name]:%s [value]:%f"), *InCVarString, *OutCommand, OutValue);
}

TMap<FString, IConsoleVariable*> FImGuiDebuggerExtension::GetCVarList(const FString& InCategory)
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
