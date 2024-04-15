#pragma once

#include "CoreMinimal.h"
#include <imgui.h>

enum EDebugSection: uint8
{
    None,
    Engine,
    Project,     // project should be private
    Num
};

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