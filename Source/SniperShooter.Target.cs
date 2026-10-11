using UnrealBuildTool;
using System.Collections.Generic;

public class SniperShooterTarget : TargetRules
{
	public SniperShooterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("SniperShooter");
	}
}
