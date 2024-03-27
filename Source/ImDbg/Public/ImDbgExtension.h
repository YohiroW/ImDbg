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
struct FImDbgEntry
{
    EDebugSection Section;
    FString Command;
    FString Args;
	bool bToggled;
    FString DisplayName;

	bool operator== (const FImDbgEntry& Other);
    void Execure();
};

class IImDbgExtension
{
public:
    virtual void ShowMenu() = 0;
};

class FImDbgExtension : public IImDbgExtension
{
public:
    FImDbgExtension();
    virtual ~FImDbgExtension();

    virtual void RegisterDebuggerEntry(const FImDbgEntry& Entry);
    virtual void UnregisterDebuggerEntry(const FImDbgEntry& Entry);

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
	TArray<FImDbgEntry> Entries;

};