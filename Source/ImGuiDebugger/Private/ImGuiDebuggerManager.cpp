#include "ImGuiDebuggerManager.h"
#include "ImGuiDebuggerExtension.h"
#include "ImGuiDebuggerEngine.h"
#include "ImGuiDebuggerProfiler.h"
#include <ImGuiModule.h>

#define WITH_IMGUI_DEBUGGER 1
#if WITH_IMGUI_DEBUGGER
#include <imgui.h>
#include <implot.h>
#endif

UImGuiDebuggerManager::UImGuiDebuggerManager(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	this->Initialize();
	bRequestShowFlagUpdate = false;
}

void UImGuiDebuggerManager::Initialize()
{
	TickDelegate = FTickerDelegate::CreateUObject(this, &UImGuiDebuggerManager::Refresh);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate, 0.0f);

	FImGuiDebuggerExtension* EngineExt = new FImGuiDebuggerEngine();
	RegisterDebuggerExtension(EngineExt);

	FImGuiDebuggerStats* StatExt = new FImGuiDebuggerStats();
	RegisterDebuggerExtension(StatExt);
}

UImGuiDebuggerManager::~UImGuiDebuggerManager()
{
	if (Extensions.Num() == 0)
	{
		return;
	}

	for (FImGuiDebuggerExtension* DebugExt : Extensions)
	{
		UnregisterDebuggerExtension(DebugExt);
	}
}

bool UImGuiDebuggerManager::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	return Super::ProcessConsoleExec(Cmd, Ar, Executor);
}

bool UImGuiDebuggerManager::ExecuteCommand(const UObject* WorldContextObject, const FString& Command)
{
	//if (!WorldContextObject)
	//{
	//	return false;
	//}

	//UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	//// First, try routing through the console manager directly.
	//// This is needed in case the Exec commands have been compiled out, meaning that GEngine->Exec wouldn't route to anywhere.
	//if (IConsoleManager::Get().ProcessUserConsoleInput(*Command, *GLog, World) == false)
	//{
	//	GEngine->Exec(World, *Command);
	//}


	return true;
}

void UImGuiDebuggerManager::RegisterDebuggerExtension(FImGuiDebuggerExtension* InExtension)
{
	Extensions.Add(InExtension);
}

void UImGuiDebuggerManager::UnregisterDebuggerExtension(FImGuiDebuggerExtension* InExtension)
{
	Extensions.Remove(InExtension);

	delete InExtension;
	InExtension = nullptr;
}

bool UImGuiDebuggerManager::Refresh(float DeltaTime)
{
#if WITH_IMGUI_DEBUGGER && !WITH_EDITOR
	// Refresh imgui overlay one by one
 
    static bool bDrawImGuiDemo = false;
	static bool bDrawImPlotDemo = false;
 
    if(ImGui::BeginMainMenuBar())
	{
		if (FImGuiModule* Module = FModuleManager::GetModulePtr<FImGuiModule>("ImGui"))
		{
			for(FImGuiDebuggerExtension* DebugExt: Extensions)
			{
				DebugExt->ShowMenu();
			}

			if (ImGui::BeginMenu("Profile"))
			{
				ImGui::Checkbox("GPU Profiler", &bDrawImPlotDemo);
				ImGui::Checkbox("CPU Profiler", &bDrawImGuiDemo);
				ImGui::EndMenu();
			}

			if (ImGui::Button("Report", ImVec2(56,22)))
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
			const int FPS = static_cast<int>(1.0f / DeltaTime);
			const float Millis = DeltaTime * 1000.0f;
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%3d FPS %.02f ms", FPS, Millis);

			ImGui::SameLine(ImGui::GetWindowWidth()- 120.0f);
#if UE_BUILD_TEST
			const char* BuildVersion = TCHAR_TO_UTF8(*FString::Printf(TEXT("CL%u_Test"), FEngineVersion::Current().GetChangelist()));
#elif UE_BUILD_DEVELOPMENT
			const char* BuildVersion = TCHAR_TO_UTF8(*FString::Printf(TEXT("CL%u_Dev"), FEngineVersion::Current().GetChangelist()));
#endif
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", BuildVersion);
		}

		ImGui::EndMainMenuBar();
    }

    if(bDrawImGuiDemo)
    {
        ImGui::ShowDemoWindow(&bDrawImGuiDemo);
    }
	if (bDrawImPlotDemo)
	{
		//ImPlot::ShowDemoWindow(&bDrawImPlotDemo);
		ShowGPUProfiler(&bDrawImPlotDemo);
	}
#endif

	return true;
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
