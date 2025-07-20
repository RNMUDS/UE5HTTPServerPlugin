using UnrealBuildTool;

public class UE5HTTPServer : ModuleRules
{
    public UE5HTTPServer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
            }
        );
        
        PrivateIncludePaths.AddRange(
            new string[] {
            }
        );
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "HTTP",
                "HTTPServer",
                "Json",
                "JsonUtilities"
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore"
            }
        );
        
        // エディタビルドの時のみUnrealEdモジュールを追加
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}