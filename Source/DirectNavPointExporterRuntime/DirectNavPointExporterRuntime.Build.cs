using UnrealBuildTool;
using System.IO;

public class DirectNavPointExporterRuntime : ModuleRules
{
	public DirectNavPointExporterRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(EngineDirectory, "Source/Runtime/Launch")
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NavigationSystem",
				"Navmesh"
			}
		);

		PublicDefinitions.AddRange(
			new string[]
			{
				"USE_DETOUR_BUILT_INTO_UE4"
			}
		);

		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
