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
	void ShowMessage();
	void Initialize();
	void Clear();
	bool IsValid(const char* InCategory, const ELogVerbosity::Type InVerbosity) const;
	void MaskAll(const bool bInLogAll);
	
	ImVec4 GetVerbosityColor(const ELogVerbosity::Type Verbosity) const;

protected:
	// All registered entries
	TArray<FImDbgEntry> Entries;

private:
	bool bEnabled = false;
	bool VerbosityChannel[ELogVerbosity::NumVerbosity];
	
	TMap<FString, bool> CategoryChannels;

	ImVector<LogItem> Items;
};