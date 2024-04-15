#pragma once

#include "ImDbgExtension.h"

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

class FImDbgEngine : public FImDbgExtension
{
public:
	FImDbgEngine();
    virtual ~FImDbgEngine();

    virtual void ShowMenu() override;

    void Initialize();

	// Engine show flags
	void InitShowFlags(const TArray<FString>& InShowFlagCommands);
	void PushShowFlagEntry(FString InConsoleCommand);

	virtual void RegisterDebuggerEntry(const FImDbgEntry& Entry);
	virtual void UnregisterDebuggerEntry(const FImDbgEntry& Entry);

private:

    void ShowEngineMenuShowFlags();
    void UpdateShowFlags();

    // Rendering
    void ShowEngineMenuRendering();
   
    // Animation
    void ShowEngineMenuAnimation();
   
    // Physics
    void ShowEngineMenuPhysics();
    
private:
	bool bRequestRefresh = false;

	// All registered entries
	TArray<FImDbgEntry> Entries;
};