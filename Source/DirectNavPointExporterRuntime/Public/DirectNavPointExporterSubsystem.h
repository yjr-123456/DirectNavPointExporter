#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DirectNavPointExporterTypes.h"
#include "DirectNavPointExporterSubsystem.generated.h"

class FDirectNavPointSampler;

struct FDirectNavCachedQueryResult
{
	TArray<FVector> Points;
	FDirectNavSamplingResult Result;
};

UCLASS()
class DIRECTNAVPOINTEXPORTERRUNTIME_API UDirectNavPointExporterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	bool ResamplePoints();

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	bool GetReachablePointsCached(
		const FDirectNavReachablePointQueryConfig& QueryConfig,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	bool GetReachablePointsInRadiusCached(
		const FDirectNavReachablePointQueryConfig& QueryConfig,
		const FDirectNavRadiusPointQueryConfig& RadiusQuery,
		TArray<FVector>& OutPoints,
		FDirectNavSamplingResult& OutResult
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	void SetSamplingOptions(
		float InStartupDelay,
		const FDirectNavSamplerConfig& InSamplerConfig,
		bool bInApplyWorldFilter,
		const FDirectNavWorldFilterConfig& InWorldFilterConfig
	);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	void ScheduleAutomaticSampling();

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	bool RefreshContext();

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	void InvalidateCache(bool bInvalidateContext = false);

	UFUNCTION(BlueprintCallable, Category = "Direct Nav Point Exporter")
	void InvalidateForWorldChange();

	UFUNCTION(BlueprintPure, Category = "Direct Nav Point Exporter")
	bool IsCacheReady() const { return bCacheReady; }

	const TArray<FVector>& GetCachedPoints() const { return CachedPoints; }

	const FDirectNavSamplingResult& GetCachedResult() const { return CachedResult; }

	UFUNCTION(BlueprintPure, Category = "Direct Nav Point Exporter")
	float GetStartupDelay() const { return StartupDelay; }

	UFUNCTION(BlueprintPure, Category = "Direct Nav Point Exporter")
	FDirectNavCacheStatus GetCacheStatus() const;

	UFUNCTION(BlueprintPure, Category = "Direct Nav Point Exporter")
	FDirectNavReachablePointQueryConfig GetDefaultQueryConfig() const;

private:
	void RunAutomaticSampling();
	bool EnsureSamplingContextReady();
	FString BuildCacheKey(const FDirectNavReachablePointQueryConfig& QueryConfig) const;
	static bool IsPointInRadius(const FVector& Point, const FDirectNavRadiusPointQueryConfig& RadiusQuery);
	static FDirectNavSamplerConfig MakeSamplerConfig(const FDirectNavReachablePointQueryConfig& QueryConfig);
	static FDirectNavWorldFilterConfig MakeWorldFilterConfig(const FDirectNavReachablePointQueryConfig& QueryConfig);
	static int32 FilterPointsWithWorldCollision(UWorld* World, FDirectNavSamplingResult& InOutResult, const FDirectNavWorldFilterConfig& FilterConfig);

private:
	UPROPERTY()
	FDirectNavSamplerConfig SamplerConfig;

	UPROPERTY()
	bool bApplyWorldFilter = true;

	UPROPERTY()
	FDirectNavWorldFilterConfig WorldFilterConfig;

	UPROPERTY()
	float StartupDelay = 0.2f;

	UPROPERTY()
	TArray<FVector> CachedPoints;

	UPROPERTY()
	FDirectNavSamplingResult CachedResult;

	UPROPERTY()
	bool bCacheReady = false;

	UPROPERTY()
	FString CachedMapName;

	UPROPERTY()
	int32 ContextVersion = 0;

	FTimerHandle StartupSamplingTimerHandle;
	FDirectNavPointSampler* CachedSampler = nullptr;
	TMap<FString, FDirectNavCachedQueryResult> QueryResultCache;
};
