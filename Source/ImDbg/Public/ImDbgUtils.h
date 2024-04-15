#pragma once

#include "CoreMinimal.h"

class  FImDbgUtil
{
public:
	FImDbgUtil() = delete;

	static TMap<FString, IConsoleVariable*> GetCVarList(const FString& InCategory);

	// eg: parse cvar like this 'showflag.staticmeshes 2'
	// OutCategory: showflag
	// OutName: staticmeshes
	// OutValue: 2
	static void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, int32& OutValue);
	static void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, FString& OutCommandName, float& OutValue);
	static void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, int32& OutValue);
	static void ParseConsoleVariable(FString& InCVarString, FString& OutCommand, float& OutValue);
	static void ParseConsoleVariable(FString& InCVarString, FString& OutCategory, FString& OutCommandDisplayName);
};