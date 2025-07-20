#include "UE5HTTPServerModule.h"
#include "UE5HTTPServer.cpp" 

#define LOCTEXT_NAMESPACE "UE5HTTPServerModule"

void UE5HTTPServerModule::StartupModule()
{
    HTTPServer = MakeShareable(new UE5HTTPServer());
    HTTPServer->StartServer();
    
    UE_LOG(LogTemp, Warning, TEXT("UE5HTTPServer Module Started"));
}

void UE5HTTPServerModule::ShutdownModule()
{
    if (HTTPServer.IsValid())
    {
        HTTPServer->StopServer();
        HTTPServer.Reset();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UE5HTTPServer Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(UE5HTTPServerModule, UE5HTTPServer)