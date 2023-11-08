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
}

void FImGuiDebuggerExtension::RegisterDebuggerEntry(const FImGuiDebugEntry& Entry)
{
	Entries.Add(Entry);
}

void FImGuiDebuggerExtension::UnregisterDebuggerEntry(const FImGuiDebugEntry& Entry)
{
	Entries.Remove(Entry);
}

void FImGuiDebuggerExtension::ShowMenu()
{
	UE_LOG(LogImGuiDebugger, Log, TEXT("To be implemented."));
}
