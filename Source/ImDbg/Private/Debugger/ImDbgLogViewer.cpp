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
	for (int32 i = 0; i< ELogVerbosity::Type::NumVerbosity; ++i)
	{
		VerbosityChannel[i] = false;
	}

	VerbosityChannel[ELogVerbosity::Warning] = true;
	VerbosityChannel[ELogVerbosity::Error] = true;
	VerbosityChannel[ELogVerbosity::Display] = true;
	VerbosityChannel[ELogVerbosity::Log] = true;

	for (auto Pair: CategoryChannels)
	{
		Pair.Value = false;
	}
}

void FImDbgLogViewer::Clear()
{
	Items.clear();
}

bool FImDbgLogViewer::IsValid(const char* InCategory, const ELogVerbosity::Type InVerbosity) const
{
	FString CategoryStr = FString(InCategory);
	bool bIsValidCategory = CategoryChannels.Contains(CategoryStr)? CategoryChannels[CategoryStr] : false;
	
	bool bIsValidVerbosity = VerbosityChannel[InVerbosity];

	return bIsValidCategory && bIsValidVerbosity;
}

void FImDbgLogViewer::MaskAll(const bool bInLogAll)
{
	for (auto Pair : CategoryChannels)
	{
		CategoryChannels[Pair.Key] = bInLogAll;
	}
}

ImVec4 FImDbgLogViewer::GetVerbosityColor(const ELogVerbosity::Type Verbosity) const
{
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:	      return ImVec4(1.0f, 0.0f, 0.5f, 1.0f);
	case ELogVerbosity::Error:        return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	case ELogVerbosity::Warning:      return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	case ELogVerbosity::Display:      return ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
	case ELogVerbosity::Log:	      return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	case ELogVerbosity::Verbose:      return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	case ELogVerbosity::VeryVerbose:  return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	default: 
		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void FImDbgLogViewer::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
{
	//if (bEnabled)
	{
		FString CategoryStr = Category.ToString();
		const char* CategoryName = Strdup(TCHAR_TO_ANSI(*Category.ToString()));
		char* Msg = Strdup(TCHAR_TO_ANSI(Message));

		if (!CategoryChannels.Contains(CategoryStr))
		{
			CategoryChannels.Add(CategoryStr, true);
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
				if (ImGui::Button("Log All"))	   MaskAll(true);
				ImGui::SameLine();
				if (ImGui::Button("Mute All"))   MaskAll(false);
				for (auto Pair: CategoryChannels)
				{
					ImGui::MenuItem(TCHAR_TO_ANSI(*Pair.Key), "", &CategoryChannels[Pair.Key]);
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

				ShowMessage();

				ImGui::EndTable();
			}
			ImGui::EndChild();

			ImGui::End();
		}
	}
}

void FImDbgLogViewer::ShowMessage()
{
	for (int32 i = 0; i < Items.Size; i++)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, GetVerbosityColor(Items[i].Verbosity));
		// Filter message
		if (IsValid(Items[i].Category, Items[i].Verbosity))
		{
			ImGui::TableNextColumn(); ImGui::TextUnformatted(Items[i].Category);
			ImGui::TableNextColumn(); ImGui::TextUnformatted(Items[i].Message);
		}
		ImGui::PopStyleColor();
	}
}
