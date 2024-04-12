#pragma once

#include "CoreMinimal.h"
#include "ImDbgExtension.h"

class FImDbgLogViewer : public FImDbgExtension
{
public:
	FImDbgLogViewer();
	virtual ~FImDbgLogViewer();

	virtual void ShowMenu() override;

	void Initialize();

private:
	bool bEnabled = false;

};