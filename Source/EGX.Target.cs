// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EGXTarget : TargetRules
{
	public EGXTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.AddRange(new string[]
		{
			"EGX",
			"EGXCore",
			"EGXEconomy",
			"EGXRTS",
			"EGXAI",
			"EGXPlanet"
		});
	}
}
