// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class myproject : ModuleRules
{
	public myproject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"myproject",
			"myproject/Variant_Platforming",
			"myproject/Variant_Platforming/Animation",
			"myproject/Variant_Combat",
			"myproject/Variant_Combat/AI",
			"myproject/Variant_Combat/Animation",
			"myproject/Variant_Combat/Gameplay",
			"myproject/Variant_Combat/Interfaces",
			"myproject/Variant_Combat/UI",
			"myproject/Variant_SideScrolling",
			"myproject/Variant_SideScrolling/AI",
			"myproject/Variant_SideScrolling/Gameplay",
			"myproject/Variant_SideScrolling/Interfaces",
			"myproject/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
