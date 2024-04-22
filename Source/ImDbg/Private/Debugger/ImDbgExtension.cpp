#include "ImDbgExtension.h"
#include "ImDbgModule.h"
#include "Kismet/GameplayStatics.h"

FImDbgExtension::FImDbgExtension()
{
}

FImDbgExtension::~FImDbgExtension()
{
	Release();
}

void FImDbgExtension::Release()
{

}

void FImDbgExtension::ShowMenu(float InDeltaTime)
{
	UE_LOG(LogImDbg, Log, TEXT("To be implemented."));
}
