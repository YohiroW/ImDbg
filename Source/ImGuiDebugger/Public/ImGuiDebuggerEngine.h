#pragma once

#include "ImGuiDebuggerExtension.h"

class FImGuiDebuggerEngine : public FImGuiDebuggerExtension
{
public:
	FImGuiDebuggerEngine();
    virtual ~FImGuiDebuggerEngine();

    virtual void ShowMenu() override;

    void Initialize();

private:
    // Engine show flags
    void InitShowFlags();
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