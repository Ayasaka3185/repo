using UnrealBuildTool;
using System.Collections.Generic;

public class Project_UI_MenuTarget : TargetRules
{
	public Project_UI_MenuTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Project_UI_Menu");
	}
}
