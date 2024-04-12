#include "ImDbgLogViewer.h"

FImDbgLogViewer::FImDbgLogViewer()
{
}

FImDbgLogViewer::~FImDbgLogViewer()
{
}

void FImDbgLogViewer::ShowMenu()
{
	if (ImGui::Button("Console"))
	{
		bEnabled = true;
	}

	if (bEnabled)
	{
		if (ImGui::Begin("Console"), &bEnabled)
		{


			ImGui::End();
		}
	}
}

void FImDbgLogViewer::Initialize()
{
}
