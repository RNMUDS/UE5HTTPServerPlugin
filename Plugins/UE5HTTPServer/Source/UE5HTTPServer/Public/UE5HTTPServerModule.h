#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUE5HTTPServer;

class FUE5HTTPServerModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TSharedPtr<FUE5HTTPServer> HTTPServer;
};