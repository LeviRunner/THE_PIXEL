using UnrealBuildTool;

public class DungeonStanceTarget : TargetRules
{
	public DungeonStanceTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("DungeonStance");
	}
}
