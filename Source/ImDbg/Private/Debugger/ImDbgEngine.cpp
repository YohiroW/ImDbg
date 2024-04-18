#include "ImDbgEngine.h"
#include "ImDbgManager.h"
#include "ImDbgUtils.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/ConsoleManager.h"
#include "ShowFlags.h"

#define IDB_ENGINE_CATRGORY          "Engine"
#define IDB_ENGINE_CATRGORY_SHOWFLAG "ShowFlags"

bool FImDbgEntry::operator== (const FImDbgEntry& Other)
{
	return this->Command == Other.Command
		&& this->Args == Other.Args
		&& this->DisplayName == Other.DisplayName;
}

void FImDbgEntry::Execure()
{
	FString Argument = bToggled ? FString("1") : FString("0");
	FString Cmd = FString::Printf(TEXT("%s %s"), *Command, *Argument);

	UKismetSystemLibrary::ExecuteConsoleCommand(GEngine->GetWorld(), Cmd);
}

FImDbgEngine::FImDbgEngine()
{
    Initialize();
}

FImDbgEngine::~FImDbgEngine()
{
	if (Entries.Num() == 0)
	{
		return;
	}

	Entries.Empty();
}

void FImDbgEngine::Initialize()
{

}

void FImDbgEngine::PushShowFlagEntry(FString InConsoleCommand)
{
	FString Category, CommandDisplayName;
	FImDbgUtil::ParseConsoleVariable(InConsoleCommand, Category, CommandDisplayName);
	int32 ShowFlagIndex = FEngineShowFlags::FindIndexByName(*CommandDisplayName);

	//FEngineShowFlags EditorShowFlags(ESFIM_Editor);
	FEngineShowFlags GameShowFlags(ESFIM_Game);

	bool bSet = GameShowFlags.GetSingleFlag(ShowFlagIndex);
	int32 Value = bSet ? 1 : 0;

	FImDbgEntry Entry;
	Entry.Command = InConsoleCommand;
	Entry.DisplayName = CommandDisplayName;
	Entry.bToggled = bSet;
	Entry.Args = FString::Printf(TEXT("%d"),Value);

	RegisterDebuggerEntry(Entry);
}

void FImDbgEngine::RegisterDebuggerEntry(const FImDbgEntry& Entry)
{
	Entries.Add(Entry);
}

void FImDbgEngine::UnregisterDebuggerEntry(const FImDbgEntry& Entry)
{
	Entries.Remove(Entry);
}

void FImDbgEngine::ShowMenu()
{
    if (ImGui::BeginMenu(IDB_ENGINE_CATRGORY))
    {
		ShowEngineMenuShowFlags();
        ShowEngineMenuRendering();
        ShowEngineMenuAnimation();
        ShowEngineMenuPhysics();
    
        ImGui::EndMenu();
    }
}

void FImDbgEngine::InitShowFlags(const TArray<FString>& InShowFlagCommands)
{
	//
    // Push some common showflags with engine default config
	// thus, might have an issue like when the default config changed
	// the show flag in debugger need trigger twice to activate..
	// 
	for (const FString& ShowFlagCommand: InShowFlagCommands)
	{
		PushShowFlagEntry(ShowFlagCommand);
	}
}

void FImDbgEngine::ShowEngineMenuShowFlags()
{
    if (ImGui::BeginMenu(IDB_ENGINE_CATRGORY_SHOWFLAG))
    {
        for (FImDbgEntry& ShowflagEntry: Entries)
        {
            if(ImGui::Checkbox(TCHAR_TO_UTF8(*ShowflagEntry.DisplayName), &(ShowflagEntry.bToggled)))
            {
				ShowflagEntry.Execure();
            }
        }
        ImGui::EndMenu();
    }
}

void FImDbgEngine::UpdateShowFlags()
{

}

void FImDbgEngine::ShowEngineMenuRendering()
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
			const char* items[] = { "B8G8R8A8", "A2B10G10R10", "FloatR11G11B10", "FloatRGB", "FloatRGBA", "A32B32G32R32F" };
			static int item_current = 4;
			ImGui::Combo("", &item_current, items, IM_ARRAYSIZE(items));

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

void FImDbgEngine::ShowEngineMenuAnimation()
{
    if (ImGui::BeginMenu("Animation"))
    {

        ImGui::EndMenu();
    }
}

void FImDbgEngine::ShowEngineMenuPhysics()
{
    if (ImGui::BeginMenu("Physics"))
    {

        ImGui::EndMenu();
    }
}
