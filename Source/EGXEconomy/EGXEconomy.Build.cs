using UnrealBuildTool;

public class EGXEconomy : ModuleRules
{
	public EGXEconomy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"EGXCore"
		});
	}
}