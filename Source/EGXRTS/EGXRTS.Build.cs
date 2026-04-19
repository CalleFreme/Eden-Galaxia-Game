using UnrealBuildTool;

public class EGXRTS : ModuleRules
{
	public EGXRTS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"EGXCore",
			"EGXPlanet",
			"EGXEconomy"
		});
	}
}