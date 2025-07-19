// Private/UE5HTTPServer.cpp

#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Camera/CameraActor.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

class FUE5HTTPServer
{
public:
    FUE5HTTPServer()
    {
        HttpServerModule = &FHttpServerModule::Get();
    }

    ~FUE5HTTPServer()
    {
        StopServer();
    }

    void StartServer()
    {
        HttpRouter = HttpServerModule->GetHttpRouter(8080);
        
        if (!HttpRouter.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create HTTP router on port 8080"));
            return;
        }

        SetupRoutes();
        HttpServerModule->StartAllListeners();
        UE_LOG(LogTemp, Warning, TEXT("HTTP Server started on port 8080"));
    }

    void StopServer()
    {
        if (HttpServerModule)
        {
            HttpServerModule->StopAllListeners();
            UE_LOG(LogTemp, Warning, TEXT("HTTP Server stopped"));
        }
    }

private:
    FHttpServerModule* HttpServerModule;
    TSharedPtr<IHttpRouter> HttpRouter;

    void SetupRoutes()
    {
        // ヘルスチェック
        HttpRouter->BindRoute(FHttpPath(TEXT("/health")), 
            EHttpServerRequestVerbs::VERB_GET,
            FHttpRequestHandler::CreateLambda(
                [](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    auto Response = FHttpServerResponse::Create(TEXT("{\"status\":\"ok\"}"), TEXT("application/json"));
                    OnComplete(MoveTemp(Response));
                    return true;
                }
            ));

        // アクター作成
        HttpRouter->BindRoute(FHttpPath(TEXT("/actors")), 
            EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateLambda(
                [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    return HandleCreateActor(Request, OnComplete);
                }
            ));

        // アクター移動
        HttpRouter->BindRoute(FHttpPath(TEXT("/actors/*/location")), 
            EHttpServerRequestVerbs::VERB_PUT,
            FHttpRequestHandler::CreateLambda(
                [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    return HandleMoveActor(Request, OnComplete);
                }
            ));

        // アクター回転
        HttpRouter->BindRoute(FHttpPath(TEXT("/actors/*/rotation")), 
            EHttpServerRequestVerbs::VERB_PUT,
            FHttpRequestHandler::CreateLambda(
                [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    return HandleRotateActor(Request, OnComplete);
                }
            ));

        // アクター色変更
        HttpRouter->BindRoute(FHttpPath(TEXT("/actors/*/color")), 
            EHttpServerRequestVerbs::VERB_PUT,
            FHttpRequestHandler::CreateLambda(
                [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    return HandleSetActorColor(Request, OnComplete);
                }
            ));

        // アクタースケール変更
        HttpRouter->BindRoute(FHttpPath(TEXT("/actors/*/scale")), 
            EHttpServerRequestVerbs::VERB_PUT,
            FHttpRequestHandler::CreateLambda(
                [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    return HandleSetActorScale(Request, OnComplete);
                }
            ));

        // アクター削除
        HttpRouter->BindRoute(FHttpPath(TEXT("/actors/*")), 
            EHttpServerRequestVerbs::VERB_DELETE,
            FHttpRequestHandler::CreateLambda(
                [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    return HandleDeleteActor(Request, OnComplete);
                }
            ));

        // シーン情報取得
        HttpRouter->BindRoute(FHttpPath(TEXT("/scene")), 
            EHttpServerRequestVerbs::VERB_GET,
            FHttpRequestHandler::CreateLambda(
                [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
                {
                    return HandleGetSceneInfo(Request, OnComplete);
                }
            ));

        UE_LOG(LogTemp, Warning, TEXT("HTTP routes configured"));
    }

    bool HandleCreateActor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
    {
        TSharedPtr<FJsonObject> JsonBody = ParseJsonBody(Request);
        if (!JsonBody.IsValid())
        {
            SendErrorResponse(OnComplete, TEXT("Invalid JSON"));
            return true;
        }

        FString ActorType = JsonBody->GetStringField(TEXT("type"));
        FString ActorName = JsonBody->GetStringField(TEXT("name"));
        
        TSharedPtr<FJsonObject> LocationObj = JsonBody->GetObjectField(TEXT("location"));
        FVector Location(
            LocationObj->GetNumberField(TEXT("x")),
            LocationObj->GetNumberField(TEXT("y")),
            LocationObj->GetNumberField(TEXT("z"))
        );

        // 色情報の取得（オプション）
        FLinearColor Color = FLinearColor::White;
        if (JsonBody->HasField(TEXT("color")))
        {
            TSharedPtr<FJsonObject> ColorObj = JsonBody->GetObjectField(TEXT("color"));
            Color = FLinearColor(
                ColorObj->GetNumberField(TEXT("r")),
                ColorObj->GetNumberField(TEXT("g")),
                ColorObj->GetNumberField(TEXT("b")),
                ColorObj->HasField(TEXT("a")) ? ColorObj->GetNumberField(TEXT("a")) : 1.0f
            );
        }

        // スケール情報の取得（オプション）
        FVector Scale = FVector(1.0f, 1.0f, 1.0f);
        if (JsonBody->HasField(TEXT("scale")))
        {
            TSharedPtr<FJsonObject> ScaleObj = JsonBody->GetObjectField(TEXT("scale"));
            if (ScaleObj->HasField(TEXT("uniform")))
            {
                float UniformScale = ScaleObj->GetNumberField(TEXT("uniform"));
                Scale = FVector(UniformScale, UniformScale, UniformScale);
            }
            else
            {
                Scale = FVector(
                    ScaleObj->GetNumberField(TEXT("x")),
                    ScaleObj->GetNumberField(TEXT("y")),
                    ScaleObj->GetNumberField(TEXT("z"))
                );
            }
        }

        UWorld* World = GetGameWorld();
        if (!World)
        {
            SendErrorResponse(OnComplete, TEXT("No active world"));
            return true;
        }

        AActor* NewActor = nullptr;
        
        if (ActorType == TEXT("Cube") || ActorType == TEXT("Sphere") || 
            ActorType == TEXT("Cylinder") || ActorType == TEXT("Plane"))
        {
            AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
            
            if (MeshActor)
            {
                FString MeshPath;
                if (ActorType == TEXT("Cube"))
                    MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
                else if (ActorType == TEXT("Sphere"))
                    MeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
                else if (ActorType == TEXT("Cylinder"))
                    MeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
                else if (ActorType == TEXT("Plane"))
                    MeshPath = TEXT("/Engine/BasicShapes/Plane.Plane");
                
                UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
                if (Mesh)
                {
                    MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
                    
                    // マテリアルの設定
                    UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
                    if (BaseMaterial)
                    {
                        UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, MeshActor);
                        DynMaterial->SetVectorParameterValue(TEXT("Color"), Color);
                        MeshActor->GetStaticMeshComponent()->SetMaterial(0, DynMaterial);
                    }
                }
                
                // スケールの設定
                MeshActor->SetActorScale3D(Scale);
                
                NewActor = MeshActor;
            }
        }
        else if (ActorType == TEXT("Light"))
        {
            APointLight* LightActor = World->SpawnActor<APointLight>(Location, FRotator::ZeroRotator);
            if (LightActor)
            {
                // ライトの色と強度設定
                UPointLightComponent* LightComponent = LightActor->PointLightComponent;
                if (LightComponent)
                {
                    LightComponent->SetLightColor(Color);
                    
                    // 強度の設定（オプション）
                    if (JsonBody->HasField(TEXT("intensity")))
                    {
                        float Intensity = JsonBody->GetNumberField(TEXT("intensity"));
                        LightComponent->SetIntensity(Intensity);
                    }
                    
                    // 減衰半径の設定（オプション）
                    if (JsonBody->HasField(TEXT("attenuationRadius")))
                    {
                        float Radius = JsonBody->GetNumberField(TEXT("attenuationRadius"));
                        LightComponent->SetAttenuationRadius(Radius);
                    }
                }
                NewActor = LightActor;
            }
        }
        else if (ActorType == TEXT("Camera"))
        {
            NewActor = World->SpawnActor<ACameraActor>(Location, FRotator::ZeroRotator);
        }

        if (NewActor)
        {
            NewActor->SetActorLabel(ActorName);
            
            UE_LOG(LogTemp, Warning, TEXT("Created actor: %s at location (%f, %f, %f) with color (%f, %f, %f)"), 
                *ActorName, Location.X, Location.Y, Location.Z, Color.R, Color.G, Color.B);
            
            TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);
            ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
            ResponseJson->SetStringField(TEXT("actorId"), NewActor->GetName());
            
            SendJsonResponse(OnComplete, ResponseJson);
        }
        else
        {
            SendErrorResponse(OnComplete, TEXT("Failed to create actor"));
        }

        return true;
    }

    bool HandleSetActorColor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
    {
        FString Path = Request.RelativePath.GetPath();
        TArray<FString> PathParts;
        Path.ParseIntoArray(PathParts, TEXT("/"), true);
        
        if (PathParts.Num() < 2)
        {
            SendErrorResponse(OnComplete, TEXT("Invalid path"));
            return true;
        }

        FString ActorName = PathParts[1];
        
        TSharedPtr<FJsonObject> JsonBody = ParseJsonBody(Request);
        if (!JsonBody.IsValid())
        {
            SendErrorResponse(OnComplete, TEXT("Invalid JSON"));
            return true;
        }

        TSharedPtr<FJsonObject> ColorObj = JsonBody->GetObjectField(TEXT("color"));
        FLinearColor NewColor(
            ColorObj->GetNumberField(TEXT("r")),
            ColorObj->GetNumberField(TEXT("g")),
            ColorObj->GetNumberField(TEXT("b")),
            ColorObj->HasField(TEXT("a")) ? ColorObj->GetNumberField(TEXT("a")) : 1.0f
        );

        AActor* FoundActor = FindActorByName(ActorName);
        
        if (FoundActor)
        {
            bool bColorSet = false;
            
            // StaticMeshActorの場合
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(FoundActor))
            {
                UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
                if (MeshComponent)
                {
                    // 既存のマテリアルを取得
                    UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(0);
                    UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(CurrentMaterial);
                    
                    // 動的マテリアルインスタンスが無い場合は作成
                    if (!DynMaterial)
                    {
                        UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
                        if (BaseMaterial)
                        {
                            DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, MeshActor);
                            MeshComponent->SetMaterial(0, DynMaterial);
                        }
                    }
                    
                    if (DynMaterial)
                    {
                        DynMaterial->SetVectorParameterValue(TEXT("Color"), NewColor);
                        bColorSet = true;
                    }
                }
            }
            // PointLightの場合
            else if (APointLight* LightActor = Cast<APointLight>(FoundActor))
            {
                if (UPointLightComponent* LightComponent = LightActor->PointLightComponent)
                {
                    LightComponent->SetLightColor(NewColor);
                    bColorSet = true;
                }
            }
            
            if (bColorSet)
            {
                UE_LOG(LogTemp, Warning, TEXT("Set color of actor: %s to (%f, %f, %f, %f)"), 
                    *ActorName, NewColor.R, NewColor.G, NewColor.B, NewColor.A);
                
                TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);
                ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
                SendJsonResponse(OnComplete, ResponseJson);
            }
            else
            {
                SendErrorResponse(OnComplete, TEXT("Actor does not support color changes"));
            }
        }
        else
        {
            SendErrorResponse(OnComplete, TEXT("Actor not found"), 404);
        }

        return true;
    }

    bool HandleSetActorScale(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
    {
        FString Path = Request.RelativePath.GetPath();
        TArray<FString> PathParts;
        Path.ParseIntoArray(PathParts, TEXT("/"), true);
        
        if (PathParts.Num() < 2)
        {
            SendErrorResponse(OnComplete, TEXT("Invalid path"));
            return true;
        }

        FString ActorName = PathParts[1];
        
        TSharedPtr<FJsonObject> JsonBody = ParseJsonBody(Request);
        if (!JsonBody.IsValid())
        {
            SendErrorResponse(OnComplete, TEXT("Invalid JSON"));
            return true;
        }

        TSharedPtr<FJsonObject> ScaleObj = JsonBody->GetObjectField(TEXT("scale"));
        FVector NewScale;
        
        if (ScaleObj->HasField(TEXT("uniform")))
        {
            float UniformScale = ScaleObj->GetNumberField(TEXT("uniform"));
            NewScale = FVector(UniformScale, UniformScale, UniformScale);
        }
        else
        {
            NewScale = FVector(
                ScaleObj->GetNumberField(TEXT("x")),
                ScaleObj->GetNumberField(TEXT("y")),
                ScaleObj->GetNumberField(TEXT("z"))
            );
        }

        AActor* FoundActor = FindActorByName(ActorName);
        
        if (FoundActor)
        {
            FoundActor->SetActorScale3D(NewScale);
            
            UE_LOG(LogTemp, Warning, TEXT("Set scale of actor: %s to (%f, %f, %f)"), 
                *ActorName, NewScale.X, NewScale.Y, NewScale.Z);
            
            TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);
            ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
            SendJsonResponse(OnComplete, ResponseJson);
        }
        else
        {
            SendErrorResponse(OnComplete, TEXT("Actor not found"), 404);
        }

        return true;
    }

    bool HandleDeleteActor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
    {
        FString Path = Request.RelativePath.GetPath();
        TArray<FString> PathParts;
        Path.ParseIntoArray(PathParts, TEXT("/"), true);
        
        if (PathParts.Num() < 2)
        {
            SendErrorResponse(OnComplete, TEXT("Invalid path"));
            return true;
        }

        FString ActorName = PathParts[1];
        AActor* FoundActor = FindActorByName(ActorName);
        
        if (FoundActor)
        {
            FoundActor->Destroy();
            
            UE_LOG(LogTemp, Warning, TEXT("Deleted actor: %s"), *ActorName);
            
            TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);
            ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
            SendJsonResponse(OnComplete, ResponseJson);
        }
        else
        {
            SendErrorResponse(OnComplete, TEXT("Actor not found"), 404);
        }

        return true;
    }

    bool HandleMoveActor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
    {
        FString Path = Request.RelativePath.GetPath();
        TArray<FString> PathParts;
        Path.ParseIntoArray(PathParts, TEXT("/"), true);
        
        if (PathParts.Num() < 2)
        {
            SendErrorResponse(OnComplete, TEXT("Invalid path"));
            return true;
        }

        FString ActorName = PathParts[1];
        
        TSharedPtr<FJsonObject> JsonBody = ParseJsonBody(Request);
        if (!JsonBody.IsValid())
        {
            SendErrorResponse(OnComplete, TEXT("Invalid JSON"));
            return true;
        }

        TSharedPtr<FJsonObject> LocationObj = JsonBody->GetObjectField(TEXT("location"));
        FVector NewLocation(
            LocationObj->GetNumberField(TEXT("x")),
            LocationObj->GetNumberField(TEXT("y")),
            LocationObj->GetNumberField(TEXT("z"))
        );

        AActor* FoundActor = FindActorByName(ActorName);
        
        if (FoundActor)
        {
            FoundActor->SetActorLocation(NewLocation);
            
            UE_LOG(LogTemp, Warning, TEXT("Moved actor: %s to location (%f, %f, %f)"), 
                *ActorName, NewLocation.X, NewLocation.Y, NewLocation.Z);
            
            TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);
            ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
            SendJsonResponse(OnComplete, ResponseJson);
        }
        else
        {
            SendErrorResponse(OnComplete, TEXT("Actor not found"), 404);
        }

        return true;
    }

    bool HandleRotateActor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
    {
        FString Path = Request.RelativePath.GetPath();
        TArray<FString> PathParts;
        Path.ParseIntoArray(PathParts, TEXT("/"), true);
        
        if (PathParts.Num() < 2)
        {
            SendErrorResponse(OnComplete, TEXT("Invalid path"));
            return true;
        }

        FString ActorName = PathParts[1];
        
        TSharedPtr<FJsonObject> JsonBody = ParseJsonBody(Request);
        if (!JsonBody.IsValid())
        {
            SendErrorResponse(OnComplete, TEXT("Invalid JSON"));
            return true;
        }

        TSharedPtr<FJsonObject> RotationObj = JsonBody->GetObjectField(TEXT("rotation"));
        FRotator NewRotation(
            RotationObj->GetNumberField(TEXT("pitch")),
            RotationObj->GetNumberField(TEXT("yaw")),
            RotationObj->GetNumberField(TEXT("roll"))
        );

        AActor* FoundActor = FindActorByName(ActorName);
        
        if (FoundActor)
        {
            FoundActor->SetActorRotation(NewRotation);
            
            UE_LOG(LogTemp, Warning, TEXT("Rotated actor: %s to rotation (Pitch: %f, Yaw: %f, Roll: %f)"), 
                *ActorName, NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll);
            
            TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);
            ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
            SendJsonResponse(OnComplete, ResponseJson);
        }
        else
        {
            SendErrorResponse(OnComplete, TEXT("Actor not found"), 404);
        }

        return true;
    }

    bool HandleGetSceneInfo(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
    {
        UWorld* World = GetGameWorld();
        if (!World)
        {
            SendErrorResponse(OnComplete, TEXT("No active world"));
            return true;
        }

        TSharedPtr<FJsonObject> SceneJson = MakeShareable(new FJsonObject);
        TArray<TSharedPtr<FJsonValue>> ActorsArray;

        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            
            if (Actor->IsA<AWorldSettings>() || Actor->GetActorLabel().IsEmpty())
                continue;
            
            TSharedPtr<FJsonObject> ActorJson = MakeShareable(new FJsonObject);
            ActorJson->SetStringField(TEXT("name"), Actor->GetActorLabel());
            ActorJson->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
            
            FVector Location = Actor->GetActorLocation();
            TSharedPtr<FJsonObject> LocationJson = MakeShareable(new FJsonObject);
            LocationJson->SetNumberField(TEXT("x"), Location.X);
            LocationJson->SetNumberField(TEXT("y"), Location.Y);
            LocationJson->SetNumberField(TEXT("z"), Location.Z);
            ActorJson->SetObjectField(TEXT("location"), LocationJson);
            
            FRotator Rotation = Actor->GetActorRotation();
            TSharedPtr<FJsonObject> RotationJson = MakeShareable(new FJsonObject);
            RotationJson->SetNumberField(TEXT("pitch"), Rotation.Pitch);
            RotationJson->SetNumberField(TEXT("yaw"), Rotation.Yaw);
            RotationJson->SetNumberField(TEXT("roll"), Rotation.Roll);
            ActorJson->SetObjectField(TEXT("rotation"), RotationJson);
            
            FVector Scale = Actor->GetActorScale3D();
            TSharedPtr<FJsonObject> ScaleJson = MakeShareable(new FJsonObject);
            ScaleJson->SetNumberField(TEXT("x"), Scale.X);
            ScaleJson->SetNumberField(TEXT("y"), Scale.Y);
            ScaleJson->SetNumberField(TEXT("z"), Scale.Z);
            ActorJson->SetObjectField(TEXT("scale"), ScaleJson);
            
            ActorsArray.Add(MakeShareable(new FJsonValueObject(ActorJson)));
        }

        SceneJson->SetArrayField(TEXT("actors"), ActorsArray);
        SceneJson->SetNumberField(TEXT("actorCount"), ActorsArray.Num());

        UE_LOG(LogTemp, Warning, TEXT("Retrieved scene info with %d actors"), ActorsArray.Num());

        SendJsonResponse(OnComplete, SceneJson);
        return true;
    }

    UWorld* GetGameWorld()
    {
        if (GEngine)
        {
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.World() && Context.WorldType == EWorldType::PIE)
                {
                    return Context.World();
                }
            }
        }
        
#if WITH_EDITOR
        if (GEditor)
        {
            return GEditor->GetEditorWorldContext().World();
        }
#endif
        
        return nullptr;
    }

    AActor* FindActorByName(const FString& ActorName)
    {
        UWorld* World = GetGameWorld();
        if (!World) return nullptr;

        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->GetActorLabel() == ActorName)
            {
                return *ActorItr;
            }
        }
        return nullptr;
    }

    TSharedPtr<FJsonObject> ParseJsonBody(const FHttpServerRequest& Request)
    {
        FString JsonString;
        if (Request.Body.Num() > 0)
        {
            JsonString = FString(Request.Body.Num(), (const char*)Request.Body.GetData());
        }
        
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
        FJsonSerializer::Deserialize(Reader, JsonObject);
        return JsonObject;
    }

    void SendJsonResponse(const FHttpResultCallback& OnComplete, TSharedPtr<FJsonObject> JsonObject)
    {
        FString ResponseString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
        
        auto Response = FHttpServerResponse::Create(ResponseString, TEXT("application/json"));
        OnComplete(MoveTemp(Response));
    }

    void SendErrorResponse(const FHttpResultCallback& OnComplete, const FString& ErrorMessage, int32 StatusCode = 400)
    {
        TSharedPtr<FJsonObject> ErrorJson = MakeShareable(new FJsonObject);
        ErrorJson->SetStringField(TEXT("error"), ErrorMessage);
        
        FString ResponseString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
        FJsonSerializer::Serialize(ErrorJson.ToSharedRef(), Writer);
        
        auto Response = FHttpServerResponse::Create(ResponseString, TEXT("application/json"));
        Response->Code = static_cast<EHttpServerResponseCodes>(StatusCode);
        OnComplete(MoveTemp(Response));
    }
};