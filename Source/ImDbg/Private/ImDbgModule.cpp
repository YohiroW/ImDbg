// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImDbgModule.h"
#include "ImDbgManager.h"

#define LOCTEXT_NAMESPACE "FImDbgModule"

DEFINE_LOG_CATEGORY(LogImDbg);

void FImDbgModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	ImDbgManager = MakeShared<FImDbgManager>();
	ImDbgManager->Initialize();
}

void FImDbgModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FImDbgModule, ImDbg)