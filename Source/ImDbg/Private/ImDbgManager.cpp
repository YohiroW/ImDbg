#include "ImGuiDebuggerManager.h"
#include "ImGuiDebuggerExtension.h"
#include "ImGuiDebuggerEngine.h"
#include "ImGuiDebuggerProfiler.h"
#include <ImGuiModule.h>
#include "Misc/FileHelper.h"

#define WITH_IMGUI_DEBUGGER 1
#if WITH_IMGUI_DEBUGGER
#include <imgui.h>
#include <implot.h>
#endif

#define INVALID_BUILD_VERSION "0000"

UImGuiDebuggerManager::UImGuiDebuggerManager(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	this->Initialize();
}

void UImGuiDebuggerManager::Initialize()
{
	LoadWhitelist();

	TickDelegate = FTickerDelegate::CreateUObject(this, &UImGuiDebuggerManager::Refresh);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate, 0.0f);

	FImGuiDebuggerEngine* EngineExt = new FImGuiDebuggerEngine();
	EngineExt->InitShowFlags(GetCommandsByCategory("ShowFlag"));
	RegisterDebuggerExtension(EngineExt);

	FImGuiDebuggerStats* StatExt = new FImGuiDebuggerStats();
	RegisterDebuggerExtension(StatExt);
}

void UImGuiDebuggerManager::InitializeImGuiStyle()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	Style.Colors[ImGuiCol_TitleBg].w = 0.5f;
	Style.Colors[ImGuiCol_WindowBg].w = 0.65f;
	Style.Colors[ImGuiCol_MenuBarBg].w = 0.65f;
}

UImGuiDebuggerManager::~UImGuiDebuggerManager()
{
	if (Extensions.Num() == 0)
	{
		return;
	}

	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	TickDelegateHandle.Reset();

	for (FImGuiDebuggerExtension* DebugExt: Extensions)
	{
		delete DebugExt;
		DebugExt = nullptr;
	}

	Extensions.Empty();
}

void UImGuiDebuggerManager::RegisterDebuggerExtension(FImGuiDebuggerExtension* InExtension)
{
	Extensions.Add(InExtension);
}

void UImGuiDebuggerManager::UnregisterDebuggerExtension(FImGuiDebuggerExtension* InExtension)
{
	Extensions.Remove(InExtension);
}

bool UImGuiDebuggerManager::Refresh(float DeltaTime)
{
#if WITH_IMGUI_DEBUGGER && !WITH_EDITOR
	if (!bIsImGuiInitialized)
	{
		if (!ImGui::GetCurrentContext())
		{
			return false;
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
	return true;
}

void UImGuiDebuggerManager::ShowMainMenu(float DeltaTime)
{
	// Refresh imgui overlay one by one
	static bool bDrawImGuiDemo = false;
	static bool bDrawImPlotDemo = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (FImGuiModule* Module = FModuleManager::GetModulePtr<FImGuiModule>("ImGui"))
		{
			for (FImGuiDebuggerExtension* DebugExt : Extensions)
			{
				DebugExt->ShowMenu();
			}

			if (ImGui::BeginMenu("Profile"))
			{
				ImGui::Checkbox("GPU Profiler", &bDrawImPlotDemo);
				ImGui::Checkbox("CPU Profiler", &bDrawImGuiDemo);
				ImGui::EndMenu();
			}

			if (ImGui::Button("Report", ImVec2(56, 22)))
			{
				UE_LOG(LogTemp, Warning, TEXT("On Report Click..."));
			}
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
}

void UImGuiDebuggerManager::ShowOverlay()
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

FVector UImGuiDebuggerManager::GetPlayerLocation()
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

void UImGuiDebuggerManager::ShowGPUProfiler(bool* bIsOpen)
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

void UImGuiDebuggerManager::LoadWhitelist(const FString& Whitelist)
{	
	// Load our white list commands
	const FString PluginContentDir = FPaths::ProjectPluginsDir() / TEXT("ImGuiDebugger/Content/");
	const FString WhiteListFilePath = PluginContentDir/ Whitelist;

	if (FFileHelper::LoadFileToStringArray(TrackedCommands, *WhiteListFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ImGuiDebugger whitelist loaded."));
	}
}

bool UImGuiDebuggerManager::IsTracked(const FString& InCommand)
{
	return TrackedCommands.Contains(InCommand);
}

TArray<FString> UImGuiDebuggerManager::GetCommandsByCategory(const FString& InCategory)
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
