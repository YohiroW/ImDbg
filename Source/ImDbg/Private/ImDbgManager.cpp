#include "ImDbgManager.h"
#include "ImDbgExtension.h"
#include "ImDbgEngine.h"
#include "ImDbgProfiler.h"
#include <ImGuiModule.h>
#include "Misc/FileHelper.h"

#define WITH_IMDBG 1
#if WITH_IMDBG
#include <imgui.h>
#include <implot.h>
#endif

#define INVALID_BUILD_VERSION "0000"

static bool GImDbGEnabled = true;
static FAutoConsoleCommand ImDbGEnabled(
	TEXT("ImDbg.Enabled"),
	TEXT("Enable ImDbg overlay."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		GImDbGEnabled = !GImDbGEnabled;
	})
);

FImDbgManager::FImDbgManager()
{
}

void FImDbgManager::Initialize()
{
	LoadWhitelist();

	TSharedPtr<FImDbgEngine> EngineExt = MakeShared<FImDbgEngine>();
	EngineExt->InitShowFlags(GetCommandsByCategory("ShowFlag"));
	RegisterDebuggerExtension(EngineExt);

	TSharedPtr<FImDbgStats> StatExt = MakeShared<FImDbgStats>();
	RegisterDebuggerExtension(StatExt);

	TSharedPtr<FImDbgProfiler> ProfilerExt = MakeShared<FImDbgProfiler>();
	ProfilerExt->Initialize();
	RegisterDebuggerExtension(ProfilerExt);
}

void FImDbgManager::InitializeImGuiStyle()
{
	ImGuiStyle* CurrentStyle = &ImGui::GetStyle();
	ImVec4* Colors = CurrentStyle->Colors;

	// color settings 
	Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	Colors[ImGuiCol_ChildBg]               = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	Colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	Colors[ImGuiCol_Border]                = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	Colors[ImGuiCol_FrameBg]               = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
	Colors[ImGuiCol_TitleBg]               = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	Colors[ImGuiCol_CheckMark]             = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	Colors[ImGuiCol_SliderGrab]            = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
	Colors[ImGuiCol_Button]                = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	Colors[ImGuiCol_ButtonActive]          = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	Colors[ImGuiCol_Header]                = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.50f, 0.50f, 0.50f, 0.80f);
	Colors[ImGuiCol_HeaderActive]          = ImVec4(0.67f, 0.67f, 0.67f, 1.00f);
	Colors[ImGuiCol_Separator]             = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
	Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	Colors[ImGuiCol_Tab]                   = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	Colors[ImGuiCol_TabHovered]            = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	Colors[ImGuiCol_TabActive]             = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	Colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	Colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

	// other settings
	CurrentStyle->ChildRounding = 0;
	CurrentStyle->FrameRounding = 0;
	CurrentStyle->PopupRounding = 0;
	CurrentStyle->ScrollbarRounding = 0;
	CurrentStyle->WindowRounding = 0;
}

ETickableTickType FImDbgManager::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

bool FImDbgManager::IsAllowedToTick() const
{
	return GImDbGEnabled;
}

void FImDbgManager::Tick(float DeltaTime)
{
#if WITH_IMDBG
	if (!bIsImGuiInitialized)
	{
		if (!ImGui::GetCurrentContext())
		{
			return;
		}
		else
		{
			bIsImGuiInitialized = true;
		}
	}

	if (!bIsDebuggerInitialized && bIsImGuiInitialized)
	{
		// To see if we can find a callback when ImGui is initialized
		InitializeImGuiStyle();
		bIsDebuggerInitialized = true;
	}

	if (bIsImGuiInitialized)
	{
		ShowMainMenu(DeltaTime);
		ShowOverlay();
	}
#endif
}

TStatId FImDbgManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FImDbgManager, STATGROUP_Tickables);
}

FImDbgManager::~FImDbgManager()
{
	if (Extensions.Num() == 0)
	{
		return;
	}

	Extensions.Empty();
}

void FImDbgManager::RegisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension)
{
	Extensions.Add(InExtension);
}

void FImDbgManager::UnregisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension)
{
	Extensions.Remove(InExtension);
}

void FImDbgManager::ShowMainMenu(float DeltaTime)
{
	// Refresh imgui overlay one by one
	static bool bDrawImGuiDemo = false;
	static bool bDrawImPlotDemo = false;
	static bool bDrawMemoryProfiler = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (FImGuiModule* Module = FModuleManager::GetModulePtr<FImGuiModule>("ImGui"))
		{
			for (TSharedPtr<FImDbgExtension> DebugExt : Extensions)
			{
				DebugExt->ShowMenu();
			}

			//if (ImGui::BeginMenu("Profile"))
			//{
			//	ImGui::Checkbox("GPU Profiler", &bDrawImPlotDemo);
			//	ImGui::Checkbox("CPU Profiler", &bDrawImGuiDemo);
			//	ImGui::Checkbox("Memory Profiler", &bDrawMemoryProfiler);
			//	ImGui::EndMenu();
			//}
		}

		if (true)
		{
			ImGui::SameLine();
			FVector Location = GetPlayerLocation();
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "X: %.01f Y: %.01f Z: %.01f", Location.X, Location.Y, Location.Z);

			ImGui::SameLine(ImGui::GetWindowWidth() - 260.0f);

			extern ENGINE_API float GAverageFPS;
			//const float FPS = GAverageFPS;
			//const float Millis = (1.0f/ FPS) * 1000.0f;
			//ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%3d FPS %.02f ms", FPS, Millis);

			const int FPS = static_cast<int>(1.0f / DeltaTime);
			const float Millis = DeltaTime * 1000.0f;
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%3d FPS %.02f ms", FPS, Millis);
	
			ImGui::SameLine(ImGui::GetWindowWidth() - 120.0f);
#if UE_BUILD_TEST
			const char* BuildVersion = TCHAR_TO_UTF8(*FString::Printf(TEXT("CL%u_Test"), FEngineVersion::Current().GetChangelist()));
#elif UE_BUILD_DEVELOPMENT
			const char* BuildVersion = TCHAR_TO_UTF8(*FString::Printf(TEXT("CL%u_Dev"), FEngineVersion::Current().GetChangelist()));
#else
			const char* BuildVersion = INVALID_BUILD_VERSION;
#endif
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", BuildVersion);
		}

		ImGui::EndMainMenuBar();
	}

	if (bDrawImGuiDemo)
	{
		ImGui::ShowDemoWindow(&bDrawImGuiDemo);
	}
	if (bDrawImPlotDemo)
	{
		//ImPlot::ShowDemoWindow(&bDrawImPlotDemo);
		ShowGPUProfiler(&bDrawImPlotDemo);
	}
	if (bDrawMemoryProfiler)
	{

	}
}

void FImDbgManager::ShowOverlay()
{
	static int location = 1;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (location >= 0)
	{
		const float PAD = 10.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
		window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
		window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
		window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		window_flags |= ImGuiWindowFlags_NoMove;
	}

	if (ImGui::Begin("Example: Simple overlay", nullptr, window_flags))
	{
		ImGui::Text("Simple overlay\n" "(right-click to change position)");
		ImGui::Separator();

		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", 1.5f, 2.0f);
		else
			ImGui::Text("Mouse Position: <invalid>");

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
			if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
			if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
			if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
			ImGui::EndPopup();
		}
	}
	ImGui::End();

}

FVector FImDbgManager::GetPlayerLocation()
{
	ULocalPlayer* Player = (GEngine && GWorld) ? GEngine->GetFirstGamePlayer(GWorld) : nullptr;
	FVector PlayerLoc = FVector::ZeroVector;
	if (Player)
	{
		APlayerController* Controller = Player->GetPlayerController(GWorld);
	
		if (Controller)
		{
			if (auto Pawn = Controller->GetPawn())
			{
				PlayerLoc = Pawn->K2_GetActorLocation();
			}
		}
	}

	return PlayerLoc;
}

// utility structure for realtime plot
struct ScrollingBuffer 
{
	int MaxSize;
	int Offset;
	ImVector<ImVec2> Data;

	ScrollingBuffer(int max_size = 2000) 
	{
		MaxSize = max_size;
		Offset = 0;
		Data.reserve(MaxSize);
	}

	void AddPoint(float x, float y) 
	{
		if (Data.size() < MaxSize)
		{
			Data.push_back(ImVec2(x, y));
		}
		else 
		{
			Data[Offset] = ImVec2(x, y);
			Offset = (Offset + 1) % MaxSize;
		}
	}

	void Erase() 
	{
		if (Data.size() > 0) 
		{
			Data.shrink(0);
			Offset = 0;
		}
	}
};

// utility structure for realtime plot
struct RollingBuffer 
{
	float Span;
	ImVector<ImVec2> Data;

	RollingBuffer() 
	{
		Span = 10.0f;
		Data.reserve(2000);
	}

	void AddPoint(float x, float y) 
	{
		float xmod = fmodf(x, Span);
		if (!Data.empty() && xmod < Data.back().x)
		{
			Data.shrink(0);
		}

		Data.push_back(ImVec2(xmod, y));
	}
};

void FImDbgManager::ShowGPUProfiler(bool* bIsOpen)
{
	if (bIsOpen)
	{
		if (ImGui::Begin("GPU Profiler"))
		{
			//ImGui::BulletText("Test fps chart..");

			static RollingBuffer rdata1, rdata2;
			static float t = 0;

			t += ImGui::GetIO().DeltaTime;

			const int FPS = static_cast<int>(1.0f / ImGui::GetIO().DeltaTime);
			const float Millis = ImGui::GetIO().DeltaTime * 1000.0f;

			rdata1.AddPoint(t, FPS);
			rdata2.AddPoint(t, Millis);

			static float history = 5.0f;
			ImGui::SliderFloat("History", &history, 1, 5.0f, "%.1f s");
			rdata1.Span = history;
			rdata2.Span = history;

			static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

			if (ImPlot::BeginPlot("##Rolling", ImVec2(0, 150)))
			{
				ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
				ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 120);
				ImPlot::PlotLine("FPS", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
				ImPlot::PlotLine("Frame time", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
				ImPlot::EndPlot();
			}

			ImGui::End();
		}
	}
}

void FImDbgManager::LoadWhitelist(const FString& Whitelist)
{	
	// Load our white list commands
	const FString PluginContentDir = FPaths::ProjectPluginsDir() / TEXT("UnrealImDbg/Content/");
	const FString WhiteListFilePath = PluginContentDir/ Whitelist;

	if (FFileHelper::LoadFileToStringArray(TrackedCommands, *WhiteListFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ImDbg whitelist loaded."));
	}
}

bool FImDbgManager::IsTracked(const FString& InCommand)
{
	return TrackedCommands.Contains(InCommand);
}

TArray<FString> FImDbgManager::GetCommandsByCategory(const FString& InCategory)
{
	TArray<FString> Commands;

	for (FString Command: TrackedCommands)
	{
		if (Command.StartsWith(InCategory))
		{
			Commands.Add(Command);
		}
	}

	return Commands;
}
