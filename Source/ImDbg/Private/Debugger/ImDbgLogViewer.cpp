#include "ImDbgLogViewer.h"
#include<imgui_internal.h>

FImDbgLogViewer::FImDbgLogViewer()
{
	GLog->AddOutputDevice(this);
}

FImDbgLogViewer::~FImDbgLogViewer()
{
	GLog->RemoveOutputDevice(this);
}

void FImDbgLogViewer::Initialize()
{
	
}

void FImDbgLogViewer::Clear()
{
	Items.clear();
}

ImColor FImDbgLogViewer::GetVerbosityColor(const ELogVerbosity::Type Verbosity) const
{
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:	      return ImColor(1.0f, 0.0f, 0.5f);
	case ELogVerbosity::Error:        return ImColor(1.0f, 0.0f, 0.0f);
	case ELogVerbosity::Warning:      return ImColor(1.0f, 1.0f, 0.0f);
	case ELogVerbosity::Display:      return ImColor(0.5f, 1.0f, 0.5f);
	case ELogVerbosity::Log:	      return ImColor(1.0f, 1.0f, 1.0f);
	case ELogVerbosity::Verbose:      return ImColor(1.0f, 1.0f, 1.0f);
	case ELogVerbosity::VeryVerbose:  return ImColor(1.0f, 1.0f, 1.0f);
	default: 
		return ImColor(1.0f, 1.0f, 1.0f);
	}
}

void FImDbgLogViewer::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
{
	//if (bEnabled)
	{
		const char* CategoryName = TCHAR_TO_ANSI(*Category.ToString());
		char* Msg = TCHAR_TO_ANSI(Message);

		if (!CategoryChannel.Contains(CategoryName))
		{
			const int32 CategoryLen = strlen(CategoryName);
			CategoryChannel.Add(CategoryName, true);
		}

		Items.push_back(LogItem(Verbosity, CategoryName, Msg));
	}
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
				Clear();
			}
			ImGui::Separator();
			// Log content
			ImGui::BeginChild("LogContent");
			if (ImGui::BeginTable("LogTable", 2, ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 100.0f);
				ImGui::TableSetupColumn("Log", ImGuiTableColumnFlags_DefaultHide);

				for (int i = 0; i < Items.Size; i++)
				{
					ImGui::TableNextColumn(); ImGui::Text(Items[i].Category);
					ImGui::TableNextColumn(); ImGui::TextColored(GetVerbosityColor(Items[i].Verbosity), Items[i].Message);
				}

				ImGui::EndTable();
			}
			ImGui::EndChild();

			ImGui::End();
		}
	}
}
