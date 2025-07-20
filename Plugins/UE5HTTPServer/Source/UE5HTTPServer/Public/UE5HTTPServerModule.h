#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UE5HTTPServer;

class UE5HTTPServerModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TSharedPtr<UE5HTTPServer> HTTPServer;
};