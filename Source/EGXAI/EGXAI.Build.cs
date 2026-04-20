using UnrealBuildTool;

public class EGXAI : ModuleRules
{
	public EGXAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"SmartObjectsModule",
			"MassEntity",
			"MassAIBehavior",
			"EGXCore",
			"EGXEconomy",
			"EGXGameplay",
			"EGXPlanet"
		});
	}
}