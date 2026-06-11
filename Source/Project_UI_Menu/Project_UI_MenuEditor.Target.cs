using UnrealBuildTool;
using System.Collections.Generic;

public class Project_UI_MenuEditorTarget : TargetRules
{
	public Project_UI_MenuEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Project_UI_Menu");
	}
}
