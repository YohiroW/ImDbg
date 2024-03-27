#pragma once

#include "CoreMinimal.h"
#include <imgui.h>

enum EDebugSection: uint8
{
    None,
    Engine,
    Project,     // project should be private
    Num
};

// Each imgui debug entry
struct FImGuiDebugEntry
{
    EDebugSection Section;
    FString Command;
    FString Args;
	bool bToggled;
    FString DisplayName;

	bool operator== (const FImGuiDebugEntry& Other);
    void Execure();
};

class IImGuiDebuggerExtension
{
public:
    virtual void ShowMenu() = 0;
};

class FImGuiDebuggerExtension : public IImGuiDebuggerExtension
{
public:
    FImGuiDebuggerExtension();
    virtual ~FImGuiDebuggerExtension();

    virtual void RegisterDebuggerEntry(const FImGuiDebugEntry& Entry);
    virtual void UnregisterDebuggerEntry(const FImGuiDebugEntry& Entry);

    virtual void Release();

    virtual void ShowMenu() override;

	// eg: parse cvar like this 'showflag.staticmeshes 2'
	// OutCategory: showflag
	// OutName: staticmeshes
	// OutValue: 2
	void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, int32& OutValue);
	void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, float& OutValue);
	void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, int32& OutValue);
	void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, float& OutValue);
	void ParseConsoleVariable(FString& InCVarString, FString& OutCategory, FString& OutCommandDisplayName);

	TMap<FString, IConsoleVariable*> GetCVarList(const FString& InCategory);

protected:
    // All registered entries
	TArray<FImGuiDebugEntry> Entries;

};