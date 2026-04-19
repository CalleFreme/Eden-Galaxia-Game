using UnrealBuildTool;

public class EGXPlanet : ModuleRules
{
	public EGXPlanet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"ProceduralMeshComponent",
			"PCG",
			"EGXCore"
		});
	}
}