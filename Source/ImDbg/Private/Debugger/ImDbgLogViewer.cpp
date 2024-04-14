#include "ImDbgLogViewer.h"
#include<imgui_internal.h>

FImDbgLogViewer::FImDbgLogViewer()
{
}

FImDbgLogViewer::~FImDbgLogViewer()
{
}

void FImDbgLogViewer::Initialize()
{
}

void FImDbgLogViewer::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
{
}

void FImDbgLogViewer::ShowMenu()
{
	if (ImGui::Button("Console"))
	{
		bEnabled = !bEnabled;
	}

	if (bEnabled)
	{
		if (ImGui::Begin("Log", &bEnabled))
		{
			if (ImGui::Button("Category"))
			{
				ImGui::OpenPopup("Category");
			}
			ImGui::SameLine();
			if (ImGui::Button("Verbosity"))
			{
				ImGui::OpenPopup("Verbosity");
			}
			ImGui::SameLine();
			if (ImGui::BeginPopup("Category"))
			{
				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				for (int32 i = 0; i< ELogVerbosity::NumVerbosity; ++i)
				{
					ImGui::MenuItem(LogVerbosityStr[i], "", &VerbosityChannel[i]);
				}
				ImGui::PopItemFlag();
				ImGui::EndPopup();
			}
			ImGui::SameLine();
			if (ImGui::BeginPopup("Verbosity"))
			{
				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				for (int32 i = 0; i < ELogVerbosity::NumVerbosity; ++i)
				{
					ImGui::MenuItem(LogVerbosityStr[i], "", &VerbosityChannel[i]);
				}
				ImGui::PopItemFlag();
				ImGui::EndPopup();
			}
			ImGui::SameLine();
			static char buf1[32] = "";  ImGui::InputText("Search", buf1, IM_ARRAYSIZE(buf1));
			ImGui::SameLine();
			if (ImGui::Button("Clear"))
			{
				
			}
			ImGui::Separator();
			// Log content
			ImGui::BeginChild("LogContent");
			if (ImGui::BeginTable("LogTable", 2, ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 100.0f);
				ImGui::TableSetupColumn("Log", ImGuiTableColumnFlags_DefaultHide);

				ImGui::TableNextColumn(); ImGui::Text("ImDbg");
				ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error...............");

				ImGui::EndTable();
			}
			ImGui::EndChild();

			ImGui::End();
		}
	}
}
