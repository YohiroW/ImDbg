#pragma once

#include "CoreMinimal.h"
#include "ImDbgExtension.h"

static char* Strdup(const char* str)
{
	size_t len = strlen(str) + 1;
	void* buf = malloc(len);
	IM_ASSERT(buf);
	return (char*)memcpy(buf, (const void*)str, len);
}

class FImDbgLogViewer : public FImDbgExtension, public FOutputDevice
{
public:
	const char* LogVerbosityStr[ELogVerbosity::NumVerbosity] =
	{
		"NoLogging", "Fatal", "Error", "Warning", "Display", "Log", "Verbose", "VeryVerbose"
	};

	struct LogItem
	{
		ELogVerbosity::Type Verbosity = ELogVerbosity::NoLogging;
		char* Category = nullptr;
		char* Message = nullptr;

		LogItem(ELogVerbosity::Type InVerbosity, const char* InCategory, char* InMessage)
		{
			Verbosity = InVerbosity;
			Category = Strdup(InCategory);
			Message = Strdup(InMessage);
		}

		~LogItem()
		{
			//free(Category);
			//free(Message);
		}
	};

public:
	FImDbgLogViewer();
	~FImDbgLogViewer();

	// Begin FOutputDevice interface
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override;
	// End FOutputDevice interface

	virtual void ShowMenu() override;

	void Initialize();
	void Clear();

	ImColor GetVerbosityColor(const ELogVerbosity::Type Verbosity) const;

private:
	bool bEnabled = false;
	bool VerbosityChannel[ELogVerbosity::NumVerbosity];
	
	TMap<const char*, bool> CategoryChannel;

	ImVector<LogItem> Items;

};