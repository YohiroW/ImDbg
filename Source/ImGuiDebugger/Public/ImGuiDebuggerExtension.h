#pragma once

#include "CoreMinimal.h"

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

    void RegisterDebuggerEntry(const FImGuiDebugEntry& Entry);
    void UnregisterDebuggerEntry(const FImGuiDebugEntry& Entry);

    virtual void ShowMenu() override;

protected:
    // All registered entries
	TArray<FImGuiDebugEntry> Entries;

};