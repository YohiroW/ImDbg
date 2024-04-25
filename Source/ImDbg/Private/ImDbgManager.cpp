#include "ImDbgManager.h"
#include "ImDbgExtension.h"
#include "ImDbgEngine.h"
#include "ImDbgProfiler.h"
#include "ImDbgLogViewer.h"
#include "ImDbgLoading.h"
#include "ImDbgSettings.h"
#include "Misc/FileHelper.h"

// From UnrealImGui plugin
#include <ImGuiModule.h>

#define WITH_IMDBG 1
#if WITH_IMDBG
#include <imgui.h>
#include <imgui_internal.h>
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
	TrackedCommands = GetDefault<UImDbgSettings>()->TrackedShowflags;

	TSharedPtr<FImDbgEngine> EngineExt = MakeShared<FImDbgEngine>();
	EngineExt->InitShowFlags(GetCommandsByCategory("ShowFlag"));
	RegisterDebuggerExtension(EngineExt);

	TSharedPtr<FImDbgProfiler> ProfilerExt = MakeShared<FImDbgProfiler>();
	ProfilerExt->Initialize();
	RegisterDebuggerExtension(ProfilerExt);

	TSharedPtr<FImDbgLoading> LoadExt = MakeShared<FImDbgLoading>();
	LoadExt->Initialize();
	RegisterDebuggerExtension(LoadExt);

	TSharedPtr<FImDbgLogViewer> LogExt = MakeShared<FImDbgLogViewer>();
	LogExt->Initialize();
	RegisterDebuggerExtension(LogExt);

	OnViewportResizedHandle = FViewport::ViewportResizedEvent.AddRaw(this, &FImDbgManager::OnViewportResized);
}

FImDbgManager::~FImDbgManager()
{
	FViewport::ViewportResizedEvent.Remove(OnViewportResizedHandle);

	if (Extensions.Num() == 0)
	{
		return;
	}

	Extensions.Empty();
}

void FImDbgManager::InitializeImGuiStyle()
{
	ImGuiIO& IO = GetImGuiIO();
	FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
	IO.DisplaySize = ImVec2(Size.X, Size.Y);
	IO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	ImGuiStyle* CurrentStyle = &ImGui::GetStyle();
	ImVec4* Colors = CurrentStyle->Colors;

	// color settings 
	Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.13f, 0.13f, 0.70f);
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
	return ETickableTickType::Conditional;
}

bool FImDbgManager::IsAllowedToTick() const
{
#if WITH_EDITOR
	bool bIsPIE = GEditor ? GEditor->IsPlayingSessionInEditor() : false;
	return GImDbGEnabled && bIsPIE;
#else
	return GImDbGEnabled;
#endif
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
			InitializeImGuiStyle();
		}
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

void FImDbgManager::RegisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension)
{
	Extensions.Add(InExtension);
}

void FImDbgManager::UnregisterDebuggerExtension(TSharedPtr<FImDbgExtension> InExtension)
{
	Extensions.Remove(InExtension);
}

void FImDbgManager::OnViewportResized(FViewport* Viewport, uint32 Unused)
{
	if (bIsImGuiInitialized)
	{
		ImGuiIO& IO = GetImGuiIO();
		FIntPoint Size = Viewport->GetSizeXY();
		IO.DisplaySize = ImVec2(Size.X, Size.Y);
		IO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}
}

void FImDbgManager::ShowMainMenu(float DeltaTime)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (FImGuiModule* Module = FModuleManager::GetModulePtr<FImGuiModule>("ImGui"))
		{
			for (TSharedPtr<FImDbgExtension> DebugExt : Extensions)
			{
				DebugExt->ShowMenu(DeltaTime);
			}
		}

		if (true)
		{
			ImGui::SameLine();
			FVector Location = FImDbgUtil::GetPlayerLocation();
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "X: %.01f Y: %.01f Z: %.01f", Location.X, Location.Y, Location.Z);

			ImGui::SameLine(ImGui::GetWindowWidth() - 260.0f);

			extern ENGINE_API float GAverageFPS;

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

	UpdateStats();

	if (ImGui::Begin("Unit Stats overlay", nullptr, window_flags))
	{
		if (ImGui::BeginTable("Unit Stats", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
		{
			ImGui::TableNextColumn(); ImGui::Text("Frame");
			ImGui::TableNextColumn(); ImGui::Text("%3.2f", FrameTime);

			ImGui::TableNextColumn(); ImGui::Text("FPS");
			ImGui::TableNextColumn(); ImGui::Text("%3.2f", round(1000 / FrameTime));

			ImGui::TableNextColumn(); ImGui::Text("Game");
			ImGui::TableNextColumn(); ImGui::Text("%3.2f", GameThreadTime);

			ImGui::TableNextColumn(); ImGui::Text("Render");
			ImGui::TableNextColumn(); ImGui::Text("%3.2f", RenderThreadTime);

			ImGui::TableNextColumn(); ImGui::Text("GPU");
			ImGui::TableNextColumn(); ImGui::Text("%3.2f", GPUFrameTime);

			ImGui::TableNextColumn(); ImGui::Text("RHI");
			ImGui::TableNextColumn(); ImGui::Text("%3.2f", RHITTime);

			ImGui::TableNextColumn(); ImGui::Text("Swap");
			ImGui::TableNextColumn(); ImGui::Text("%3.2f", SwapBufferTime);

			ImGui::TableNextColumn(); ImGui::Text("Draws");
			ImGui::TableNextColumn(); ImGui::Text("%d", GNumDrawCallsRHI[0]);

			ImGui::TableNextColumn(); ImGui::Text("Prims");
			if (GNumPrimitivesDrawnRHI[0] < 10000) 
			{
				ImGui::TableNextColumn(); ImGui::Text("%i", GNumPrimitivesDrawnRHI[0]);
			}
			else 
			{
				ImGui::TableNextColumn(); ImGui::Text("%.1f K", GNumPrimitivesDrawnRHI[0] / 1000.f);
			}

			//ImGui::TableNextRow();
			//ImGui::TableSetColumnIndex(0);
			//ImGui::Text("Input");
			//ImGui::TableSetColumnIndex(1);
			//ImGui::Text("%3.2f", InputLatencyTime);

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
			ImGui::EndTable();
		}
		ImGui::End();
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

void FImDbgManager::UpdateStats()
{
	float DiffTime;
	if (FApp::IsBenchmarking() || FApp::UseFixedTimeStep())
	{
		/** If we're in fixed time step mode, FApp::GetCurrentTime() will be incorrect for benchmarking */
		const double CurrentTime = FPlatformTime::Seconds();
		if (LastTime == 0)
		{
			LastTime = CurrentTime;
		}
		DiffTime = CurrentTime - LastTime;
		LastTime = CurrentTime;
	}
	else
	{
		/** Use the DiffTime we computed last frame, because it correctly handles the end of frame idling and corresponds better to the other unit times. */
		DiffTime = FApp::GetCurrentTime() - FApp::GetLastTime();
	}

	FrameTime = 0.9 * FrameTime + 0.1 * DiffTime * 1000.0f;
	/** Number of milliseconds the gamethread was used last frame. */
	GameThreadTime = 0.9 * GameThreadTime + 0.1 * FPlatformTime::ToMilliseconds(GGameThreadTime);
	/** Number of milliseconds the renderthread was used last frame. */
	RenderThreadTime = 0.9 * RenderThreadTime + 0.1 * FPlatformTime::ToMilliseconds(GRenderThreadTime);
	RHITTime = 0.9 * RHITTime + 0.1 * FPlatformTime::ToMilliseconds(GRHIThreadTime);
	InputLatencyTime = 0.9 * InputLatencyTime + 0.1 * FPlatformTime::ToMilliseconds64(GInputLatencyTime);
	GPUFrameTime = 0.9 * GPUFrameTime + 0.1 * FPlatformTime::ToMilliseconds(GGPUFrameTime);
	SwapBufferTime = 0.9 * SwapBufferTime + 0.1 * FPlatformTime::ToMilliseconds(GSwapBufferTime);
}

ImGuiIO& FImDbgManager::GetImGuiIO() const
{
	ImGuiContext* Context = ImGui::GetCurrentContext();
	return Context->IO;
}