#pragma once

#include "CoreMinimal.h"
#include "ImDbgExtension.h"

class FImDbgLogViewer : public FImDbgExtension, public FOutputDevice
{
public:
	const char* LogVerbosityStr[ELogVerbosity::NumVerbosity] =
	{
		"NoLogging", "Fatal", "Error", "Warning", "Display", "Log", "Verbose", "VeryVerbose"
	};

public:
	FImDbgLogViewer();
	~FImDbgLogViewer();

	// Begin FOutputDevice interface
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override;
	// End FOutputDevice interface

	virtual void ShowMenu() override;

	void Initialize();

private:
	bool bEnabled = false;

	bool VerbosityChannel[ELogVerbosity::NumVerbosity];


};