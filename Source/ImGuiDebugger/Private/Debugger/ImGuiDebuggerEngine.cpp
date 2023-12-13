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
	//
    // Push some common showflags with engine default config
	// thus, might have an issue like when the default config changed
	// the show flag in debugger need trigger twice to activate..
	// 
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Bounds", "Bounds", false);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.StaticMeshes", "StaticMesh", true);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.InstancedStaticMeshes", "InstancedStaticMeshes", true);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.InstancedFoliage", "Foliage", true);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.InstancedGrass", "Grass", true);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.HISMCOcclusionBounds", "HISMCOcclusionBounds", false);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.SkeletalMeshes", "SkeletalMesh", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Bones", "Bone", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Collision", "Collision", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.CollisionPawn", "CollisionPawn", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.CollisionVisibility", "CollisionVisibility", false);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Decals", "Decal", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.DynamicShadows", "Shadow", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.ShadowFrustums", "ShadowFrustums", false);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.MotionBlur", "MotionBlur", true);
    PUSH_ENTRY_SHOWFLAG("ShowFlag.Lighting", "Lighting", true);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.LightComplexity", "LightComplexity", false);
	PUSH_ENTRY_SHOWFLAG("ShowFlag.Particles", "Particles", true);
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
		{
			ImGui::SeparatorText("General");

			ImGui::BulletText("Resolution"); ImGui::SameLine(180.0f);
			static float Res = 100.0f;
			ImGui::SliderFloat("%", &Res, 0.0f, 100.0f, "Resolution = %.2f");
			//ImGui::Separator();

			ImGui::BulletText("Streaming Pool Size"); ImGui::SameLine(180.0f);
			static float PoolSize = 512.0f;
			if (ImGui::InputFloat("MB", &PoolSize, 256.0f, 512.0f, "%.2f"))
			{
				PoolSize = FMath::Clamp(PoolSize, 0.0f, 4096.0f);
			}

			ImGui::BulletText("SceneColor Format"); ImGui::SameLine(180.0f);
			const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
			static int item_current = 0;
			ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items));

            ImGui::BulletText("Shadow"); ImGui::SameLine(180.0f);
			static int e = 0;
			ImGui::RadioButton("Off##Shadow", &e, 0); ImGui::SameLine();
			ImGui::RadioButton("Low##Shadow", &e, 1); ImGui::SameLine();
			ImGui::RadioButton("Mid##Shadow", &e, 2); ImGui::SameLine();
			ImGui::RadioButton("High##Shadow", &e, 3); ImGui::SameLine();
			ImGui::RadioButton("Epic##Shadow", &e, 4);
            //ImGui::Separator();

			ImGui::BulletText("Anti Aliasing"); ImGui::SameLine(180.0f);
			static int e3 = 0;
			ImGui::RadioButton("Off##AA", &e3, 0); ImGui::SameLine();
			ImGui::RadioButton("FXAA##AA", &e3, 1); ImGui::SameLine();
			ImGui::RadioButton("TAA##AA", &e3, 2); ImGui::SameLine();
			ImGui::RadioButton("MSAA##AA", &e3, 3); ImGui::SameLine();
			ImGui::RadioButton("TSR##AA", &e3, 4); 
			//ImGui::Separator();

			ImGui::BulletText("GI"); ImGui::SameLine(180.0f);
			static int e2 = 0;
			ImGui::RadioButton("Off##GI", &e2, 0); ImGui::SameLine();
			ImGui::RadioButton("Lumen##GI", &e2, 1); ImGui::SameLine();
			ImGui::RadioButton("SSGI##GI", &e2, 2); ImGui::SameLine();
			ImGui::RadioButton("Ray Trace(Deprecated)##GI", &e2, 3);
			//ImGui::Separator();

			ImGui::BulletText("Reflection"); ImGui::SameLine(180.0f);
			static int e4 = 0;
			ImGui::RadioButton("Off##Reflection", &e4, 0); ImGui::SameLine();
			ImGui::RadioButton("Lumen##Reflection", &e4, 1); ImGui::SameLine();
			ImGui::RadioButton("SSR##Reflection", &e4, 2); ImGui::SameLine();
			ImGui::RadioButton("Ray Trace(Deprecated)##Reflection", &e4, 3);
			//ImGui::Separator();
		}

		{
			ImGui::SeparatorText("Nanite");

			ImGui::BulletText("Nanite"); ImGui::SameLine(180.0f);
			static int e5 = 0;
			ImGui::RadioButton("Off##Nanite", &e5, 0); ImGui::SameLine();
			ImGui::RadioButton("On##Nanite", &e5, 1); 
			//ImGui::Separator();

			//ImGui::BulletText("Stats"); ImGui::SameLine(180.0f);
			//static int e9 = 0;
			//ImGui::RadioButton("Off##Nanite", &e5, 0); ImGui::SameLine();
			//ImGui::RadioButton("On##Nanite", &e5, 1);
			//ImGui::Separator();
		}

		{
			ImGui::SeparatorText("Lumen");

			ImGui::BulletText("Lumen"); ImGui::SameLine(180.0f);
			static int e6 = 0;
			ImGui::RadioButton("Off##Lumen", &e6, 0); ImGui::SameLine();
			ImGui::RadioButton("On##Lumen", &e6, 1);

			ImGui::BulletText("LumenReflection"); ImGui::SameLine(180.0f);
			static int e7 = 0;
			ImGui::RadioButton("Off##LumenReflection", &e7, 0); ImGui::SameLine();
			ImGui::RadioButton("On##LumenReflection", &e7, 1);
		}

		{
			ImGui::SeparatorText("Lighting");

			ImGui::BulletText("Virtual Shadow Map"); ImGui::SameLine(180.0f);
			static int e8 = 0;
			ImGui::RadioButton("Off##L", &e8, 0); ImGui::SameLine();
			ImGui::RadioButton("On##L", &e8, 1);
		}

		{
			ImGui::SeparatorText("Shadow");

			ImGui::BulletText("Virtual Shadow Map"); ImGui::SameLine(180.0f);
			static int e8 = 0;
			ImGui::RadioButton("Off##VSM", &e8, 0); ImGui::SameLine();
			ImGui::RadioButton("On##VSM", &e8, 1);
		}

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
