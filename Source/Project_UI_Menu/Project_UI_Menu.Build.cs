using UnrealBuildTool;

public class Project_UI_Menu : ModuleRules
{
	public Project_UI_Menu(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"MediaAssets",
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
