#include "ImDbgProfiler.h"
#include "ImDbgModule.h"
#include "Engine/TextureCube.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureLODSettings.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "RenderTargetPool.h"
#include "Stats/StatsData.h"
#include <imgui_internal.h>
#include <implot.h>

#define LOCTEXT_NAMESPACE "ImDbg"

#define IDB_PROFILER_CATRGORY "Profiler"
#define ToMegaByte(_BYTE_) (_BYTE_ / 1024.0f / 1024.0f )


FImDbgProfiler::FImDbgProfiler()
{
}

FImDbgProfiler::~FImDbgProfiler()
{
}

void FImDbgProfiler::Initialize()
{
	GPUProfiler = MakeShared<FImDbgGPUProfiler>(&bShowGPUProfiler);
	MemoryProfiler = MakeShared<FImDbgMemoryProfiler>(&bShowMemoryProfiler);
}

void FImDbgProfiler::ShowMenu()
{
	if (ImGui::BeginMenu(IDB_PROFILER_CATRGORY))
	{
		ImGui::SeparatorText("Trace");

		if (ImGui::Button("Start Trace"))
		{

		}
		ImGui::SameLine();
		if (ImGui::Button("Stop Trace"))
		{

		}
		ImGui::SameLine();
		if (ImGui::BeginMenu("Trace Channel"))
		{
			for (int32 i = 0; i < EImDbgTraceChannel::Num; i++)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				ImGui::MenuItem(ImDbgTraceChannelName[i], "", &TraceChannels[i]);
				ImGui::PopItemFlag();
			}
			ImGui::EndMenu();
		}

		ImGui::SeparatorText("Profiler");

		ImGui::Checkbox("CPU Profiler", &bShowCPUProfiler); ImGui::SameLine();
		ImGui::Checkbox("GPU Profiler", &bShowGPUProfiler); ImGui::SameLine();
		ImGui::Checkbox("Memory Profiler", &bShowMemoryProfiler);
		ImGui::EndMenu();
	}

	if (bShowCPUProfiler)
	{
		ImPlot::ShowDemoWindow(&bShowCPUProfiler);
	}

	if (bShowGPUProfiler && GPUProfiler)
	{
		GPUProfiler->ShowMenu();
	}

	if (bShowMemoryProfiler && MemoryProfiler)
	{
		MemoryProfiler->ShowMenu();
	}
}

#undef LOCTEXT_NAMESPACE
