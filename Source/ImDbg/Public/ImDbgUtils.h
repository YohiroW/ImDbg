#pragma once

#include "CoreMinimal.h"

namespace ImDbg
{
	struct FPlotScrollingData
	{
		int32 MaxSize;
		int32 Offset;
		ImVector<ImVec2> Data;

		FPlotScrollingData(int32 InMaxSize = 1200)
		{
			MaxSize = InMaxSize;
			Offset = 0;
			Data.reserve(MaxSize);
		}

		void AddPoint(float x, float y)
		{
			if (Data.size() < MaxSize)
			{
				Data.push_back(ImVec2(x, y));
			}
			else
			{
				Data[Offset] = ImVec2(x, y);
				Offset = (Offset + 1) % MaxSize;
			}
		}

		void Erase()
		{
			if (Data.size() > 0)
			{
				Data.shrink(0);
				Offset = 0;
			}
		}
	};
}

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

	static FVector GetPlayerLocation();
};