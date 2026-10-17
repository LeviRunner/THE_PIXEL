using UnrealBuildTool;

// Servidor dedicado. Só compila com a engine buildada a partir do código-fonte
// (a versão do Epic Launcher não gera alvos de servidor).
public class DungeonStanceServerTarget : TargetRules
{
	public DungeonStanceServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("DungeonStance");
	}
}
