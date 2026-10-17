using UnrealBuildTool;

public class DungeonStanceEditorTarget : TargetRules
{
	public DungeonStanceEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("DungeonStance");
	}
}
