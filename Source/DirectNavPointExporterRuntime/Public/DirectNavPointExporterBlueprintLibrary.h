#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectNavPointExporterSubsystem.h"
#include "DirectNavPointExporterTypes.h"
#include "DirectNavPointExporterBlueprintLibrary.generated.h"

UCLASS()
class DIRECTNAVPOINTEXPORTERRUNTIME_API UDirectNavPointExporterBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "SamplerConfig,WorldFilterConfig"))
	static bool SampleFreePointsFromWorldNavMesh(
		UObject* WorldContextObject,
		const FDirectNavSamplerConfig& SamplerConfig,
		bool bApplyWorldFilter,
		const FDirectNavWorldFilterConfig& WorldFilterConfig,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Region,SamplerConfig,WorldFilterConfig"))
	static bool SampleFreePointsInRegionFromWorldNavMesh(
		UObject* WorldContextObject,
		const FBox& Region,
		const FDirectNavSamplerConfig& SamplerConfig,
		bool bApplyWorldFilter,
		const FDirectNavWorldFilterConfig& WorldFilterConfig,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool HasValidWorldNavMesh(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool GetWorldNavMeshBounds(UObject* WorldContextObject, FBox& OutBounds);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Direct Nav Point Exporter")
	static TArray<FVector> GetValidPoints(const FDirectNavSamplingResult& Result);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool IsFreePointCacheReady(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "QueryConfig"))
	static bool GetReachablePointsCached(
		UObject* WorldContextObject,
		const FDirectNavReachablePointQueryConfig& QueryConfig,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "QueryConfig"))
	static bool BuildReachableAreaCached(
		UObject* WorldContextObject,
		const FDirectNavReachablePointQueryConfig& QueryConfig,
		FDirectNavReachableAreaData& OutArea
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "QueryConfig,RadiusQuery"))
	static bool GetReachablePointsInRadiusCached(
		UObject* WorldContextObject,
		const FDirectNavReachablePointQueryConfig& QueryConfig,
		const FDirectNavRadiusPointQueryConfig& RadiusQuery,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "QueryConfig,RadiusQuery,DisplayConfig"))
	static bool ShowReachableAreaCached(
		UObject* WorldContextObject,
		const FDirectNavReachablePointQueryConfig& QueryConfig,
		const FDirectNavRadiusPointQueryConfig& RadiusQuery,
		const FDirectNavReachableAreaDisplayConfig& DisplayConfig,
		int32& OutDrawnCellCount
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool GetDefaultCachedFreePointsFromWorldNavMesh(
		UObject* WorldContextObject,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "RadiusQuery"))
	static bool GetDefaultCachedFreePointsInRadius(
		UObject* WorldContextObject,
		const FDirectNavRadiusPointQueryConfig& RadiusQuery,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "RadiusQuery,DisplayConfig"))
	static bool ShowDefaultCachedReachableArea(
		UObject* WorldContextObject,
		const FDirectNavRadiusPointQueryConfig& RadiusQuery,
		const FDirectNavReachableAreaDisplayConfig& DisplayConfig,
		int32& OutDrawnCellCount
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool RefreshFreePointCache(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool InvalidateFreePointCache(UObject* WorldContextObject, bool bInvalidateContext = false);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "SamplerConfig,WorldFilterConfig"))
	static bool ConfigureFreePointAutoSampling(
		UObject* WorldContextObject,
		float StartupDelay,
		const FDirectNavSamplerConfig& SamplerConfig,
		bool bApplyWorldFilter,
		const FDirectNavWorldFilterConfig& WorldFilterConfig
	);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool GetFreePointCacheStatus(UObject* WorldContextObject, FDirectNavCacheStatus& OutStatus);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static UDirectNavPointExporterSubsystem* GetDirectNavPointExporterSubsystem(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter", meta = (WorldContext = "WorldContextObject"))
	static bool ClearReachableAreaDebug(UObject* WorldContextObject);

private:
	static int32 DrawReachablePoints(
		UWorld* World,
		const TArray<FVector>& Points,
		const FDirectNavReachableAreaDisplayConfig& DisplayConfig
	);
	static int32 FilterPointsWithWorldCollision(UWorld* World, FDirectNavSamplingResult& InOutResult, const FDirectNavWorldFilterConfig& FilterConfig);
	static bool ResolveWorld(UObject* WorldContextObject, UWorld*& OutWorld);
};
