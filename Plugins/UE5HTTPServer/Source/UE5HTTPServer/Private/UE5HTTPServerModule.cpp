#include "UE5HTTPServerModule.h"
#include "UE5HTTPServer.cpp"

#define LOCTEXT_NAMESPACE "FUE5HTTPServerModule"

void FUE5HTTPServerModule::StartupModule()
{
    HTTPServer = MakeShareable(new FUE5HTTPServer());
    HTTPServer->StartServer();
    
    UE_LOG(LogTemp, Warning, TEXT("UE5HTTPServer Module Started"));
}

void FUE5HTTPServerModule::ShutdownModule()
{
    if (HTTPServer.IsValid())
    {
        HTTPServer->StopServer();
        HTTPServer.Reset();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UE5HTTPServer Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUE5HTTPServerModule, UE5HTTPServer)