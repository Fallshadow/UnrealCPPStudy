// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FBXImporterUtilPlugin : ModuleRules
{
	public FBXImporterUtilPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...

			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"CoreUObject",
                "Engine",
                "Json",
                "JsonUtilities"
            }
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				//"Slate",
				//"SlateCore",
				// ... add private dependencies that you statically link with here ...	
                "FBX",  // 关键：启用 FBX SDK 模块
            }
		);

        if (Target.bBuildEditor) {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
				"UnrealEd",
				"Kismet",
				"KismetCompiler",
				"AssetRegistry"
            });
        }


        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
