#pragma once

#include "ImDbgExtension.h"

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
};