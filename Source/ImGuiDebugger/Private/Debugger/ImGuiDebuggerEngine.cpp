#include "ImGuiDebuggerEngine.h"
#include "Kismet/GameplayStatics.h"

#define PUSH_ENTRY_SHOWFLAG(_CMD_,_DISPLAY_,_ARG_)\
		{ FImGuiDebugEntry Entry;\
		Entry.Command = FString(_CMD_);\
		Entry.DisplayName = FString(_DISPLAY_);\
		Entry.Section = EDebugSection::Engine;\
		Entry.bToggled = _ARG_;\
		Entry.Args = FString("1");\
		RegisterDebuggerEntry(Entry); }

FImGuiDebuggerEngine::FImGuiDebuggerEngine()
{
    Initialize();
}

FImGuiDebuggerEngine::~FImGuiDebuggerEngine()
{
}

void FImGuiDebuggerEngine::Initialize()
{
    InitShowFlags();
}

void FImGuiDebuggerEngine::ShowMenu()
{
    if (ImGui::BeginMenu("Engine"))
    {
		ShowEngineMenuShowFlags();
        ShowEngineMenuRendering();
        ShowEngineMenuAnimation();
        ShowEngineMenuPhysics();
    
        ImGui::EndMenu();
    }
}

void FImGuiDebuggerEngine::InitShowFlags()
{
    // Push some common showflags
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Bounds", "Bounds", false);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.StaticMeshes", "StaticMesh", true);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.SkeletalMeshes", "SkeletalMesh", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Bones", "Bone", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Collision", "Collision", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.CollisionPawn", "CollisionPawn", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.CollisionVisibility", "CollisionVisibility", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Decals", "Decal", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.DynamicShadows", "Shadow", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.ShadowFrustums", "ShadowFrustums", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Lighting", "Lighting", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Refraction", "Refraction", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.GlobalIllumination", "GI", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.LODColoration", "LOD", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.HLODColoration", "HLOD", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.AntiAliasing", "AntiAliasing", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.AmbientOcclusion", "AmbientOcclusion", true); 
    PUSH_ENTRY_SHOWFLAG("ShowFlag.PostProcessing", "PostProcess", true);
}

void FImGuiDebuggerEngine::ShowEngineMenuShowFlags()
{
    if (ImGui::BeginMenu("ShowFlags"))
    {
        for (FImGuiDebugEntry& DebugEntry: Entries)
        {
            if(ImGui::Checkbox(TCHAR_TO_UTF8(*DebugEntry.DisplayName), &(DebugEntry.bToggled)))
            {
                DebugEntry.Execure();
            }
        }
        ImGui::EndMenu();
    }

    //UpdateShowFlags();
}

void FImGuiDebuggerEngine::UpdateShowFlags()
{
	if (bRequestRefresh)
	{
		for (FImGuiDebugEntry& DebugEntry : Entries)
		{  
            
		}

        bRequestRefresh = false;
	}
}

void FImGuiDebuggerEngine::ShowEngineMenuRendering()
{
    if (ImGui::BeginMenu("Rendering"))
    {

        ImGui::EndMenu();
    }
}

void FImGuiDebuggerEngine::ShowEngineMenuAnimation()
{
    if (ImGui::BeginMenu("Animation"))
    {

        ImGui::EndMenu();
    }
}

void FImGuiDebuggerEngine::ShowEngineMenuPhysics()
{
    if (ImGui::BeginMenu("Physics"))
    {

        ImGui::EndMenu();
    }
}
