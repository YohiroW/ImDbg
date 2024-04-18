#pragma once

#include "CoreMinimal.h"
#include <imgui.h>

class IImDbgExtension
{
public:
    virtual void ShowMenu() = 0;
};

class FImDbgExtension : public IImDbgExtension
{
public:
    FImDbgExtension();
    virtual ~FImDbgExtension();

    virtual void Release();

    virtual void ShowMenu() override;
};