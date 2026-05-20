#include "DirectNavPointExporterSubsystem.h"

#include "DirectNavPointExporterRuntime.h"
#include "DirectNavPointSampler.h"
#include "Engine/World.h"
#include "TimerManager.h"

bool UDirectNavPointExporterSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UDirectNavPointExporterSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	CachedMapName = InWorld.GetMapName();
	InvalidateCache(true);
	ScheduleAutomaticSampling();
}

void UDirectNavPointExporterSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StartupSamplingTimerHandle);
	}

	InvalidateCache(true);
	Super::Deinitialize();
}

bool UDirectNavPointExporterSubsystem::ResamplePoints()
{
	return GetReachablePointsCached(GetDefaultQueryConfig(), CachedPoints, CachedResult);
}

bool UDirectNavPointExporterSubsystem::GetReachablePointsCached(
	const FDirectNavReachablePointQueryConfig& QueryConfig,
	TArray<FVector>& OutPoints,
	FDirectNavSamplingResult& OutResult)
{
	OutPoints.Empty();
	OutResult.Reset();

	if (!EnsureSamplingContextReady())
	{
		return false;
	}

	const FString CacheKey = BuildCacheKey(QueryConfig);
	if (const FDirectNavCachedQueryResult* CachedEntry = QueryResultCache.Find(CacheKey))
	{
		OutPoints = CachedEntry->Points;
		OutResult = CachedEntry->Result;
		CachedPoints = OutPoints;
		CachedResult = OutResult;
		bCacheReady = true;
		return true;
	}

	UWorld* World = GetWorld();
	if (!World || CachedSampler == nullptr)
	{
		return false;
	}

	FDirectNavSamplingResult NewResult;
	if (!CachedSampler->SampleGridPoints(MakeSamplerConfig(QueryConfig), NewResult))
	{
		return false;
	}

	if (QueryConfig.bApplyWorldFilter)
	{
		FilterPointsWithWorldCollision(World, NewResult, MakeWorldFilterConfig(QueryConfig));
	}

	TArray<FVector> NewPoints;
	NewPoints.Reserve(NewResult.ValidPoints);
	for (const FDirectNavSamplePoint& Point : NewResult.SamplePoints)
	{
		if (Point.bIsValid)
		{
			NewPoints.Add(Point.Location);
		}
	}

	FDirectNavCachedQueryResult CachedEntry;
	CachedEntry.Points = NewPoints;
	CachedEntry.Result = NewResult;
	QueryResultCache.Add(CacheKey, CachedEntry);

	OutPoints = MoveTemp(NewPoints);
	OutResult = MoveTemp(NewResult);
	CachedPoints = OutPoints;
	CachedResult = OutResult;
	bCacheReady = true;
	return true;
}

void UDirectNavPointExporterSubsystem::SetSamplingOptions(
	float InStartupDelay,
	const FDirectNavSamplerConfig& InSamplerConfig,
	bool bInApplyWorldFilter,
	const FDirectNavWorldFilterConfig& InWorldFilterConfig)
{
	StartupDelay = FMath::Max(0.0f, InStartupDelay);
	SamplerConfig = InSamplerConfig;
	bApplyWorldFilter = bInApplyWorldFilter;
	WorldFilterConfig = InWorldFilterConfig;
	QueryResultCache.Empty();
	bCacheReady = false;
}

void UDirectNavPointExporterSubsystem::ScheduleAutomaticSampling()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(StartupSamplingTimerHandle);
	if (StartupDelay <= 0.0f)
	{
		RunAutomaticSampling();
		return;
	}

	World->GetTimerManager().SetTimer(
		StartupSamplingTimerHandle,
		this,
		&UDirectNavPointExporterSubsystem::RunAutomaticSampling,
		StartupDelay,
		false
	);
}

bool UDirectNavPointExporterSubsystem::RefreshContext()
{
	InvalidateCache(true);
	return EnsureSamplingContextReady();
}

void UDirectNavPointExporterSubsystem::InvalidateCache(bool bInvalidateContext)
{
	QueryResultCache.Empty();
	CachedPoints.Empty();
	CachedResult.Reset();
	bCacheReady = false;

	if (bInvalidateContext)
	{
		delete CachedSampler;
		CachedSampler = nullptr;
	}
}

void UDirectNavPointExporterSubsystem::InvalidateForWorldChange()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StartupSamplingTimerHandle);
		CachedMapName = World->GetMapName();
	}

	InvalidateCache(true);
}

FDirectNavCacheStatus UDirectNavPointExporterSubsystem::GetCacheStatus() const
{
	FDirectNavCacheStatus Status;
	Status.bContextReady = CachedSampler != nullptr;
	Status.bHasDefaultResult = bCacheReady;
	Status.MapName = CachedMapName;
	Status.CachedQueryCount = QueryResultCache.Num();
	Status.CachedPointCount = CachedPoints.Num();
	Status.ContextVersion = ContextVersion;
	return Status;
}

FDirectNavReachablePointQueryConfig UDirectNavPointExporterSubsystem::GetDefaultQueryConfig() const
{
	FDirectNavReachablePointQueryConfig DefaultQueryConfig;
	DefaultQueryConfig.GridSpacing = SamplerConfig.GridSpacing;
	DefaultQueryConfig.ValidationExtent = SamplerConfig.ValidationExtent;
	DefaultQueryConfig.bApplyWorldFilter = bApplyWorldFilter;
	DefaultQueryConfig.bFilterUnderOverhangs = WorldFilterConfig.bFilterUnderOverhangs;
	DefaultQueryConfig.MinClearHeightAbove = WorldFilterConfig.MinClearHeightAbove;
	DefaultQueryConfig.OverhangTraceDistance = WorldFilterConfig.OverhangTraceDistance;
	DefaultQueryConfig.OverhangTraceChannel = WorldFilterConfig.OverhangTraceChannel;
	DefaultQueryConfig.bFilterOnObstacles = WorldFilterConfig.bFilterOnObstacles;
	DefaultQueryConfig.ObstacleTraceChannel = WorldFilterConfig.ObstacleTraceChannel;
	return DefaultQueryConfig;
}

void UDirectNavPointExporterSubsystem::RunAutomaticSampling()
{
	ResamplePoints();
}

bool UDirectNavPointExporterSubsystem::EnsureSamplingContextReady()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Sampling context init failed: invalid world"));
		InvalidateCache(true);
		return false;
	}

	const FString CurrentMapName = World->GetMapName();
	if (CachedSampler != nullptr && CachedMapName == CurrentMapName)
	{
		return true;
	}

	InvalidateCache(true);
	CachedMapName = CurrentMapName;
	CachedSampler = new FDirectNavPointSampler();

	if (!CachedSampler->Initialize(World))
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Sampling context init failed for map %s"), *CurrentMapName);
		delete CachedSampler;
		CachedSampler = nullptr;
		return false;
	}

	++ContextVersion;
	UE_LOG(LogDirectNavPointExporter, Log, TEXT("Sampling context ready for map %s, version %d"), *CurrentMapName, ContextVersion);
	return true;
}

FString UDirectNavPointExporterSubsystem::BuildCacheKey(const FDirectNavReachablePointQueryConfig& QueryConfig) const
{
	return FString::Printf(
		TEXT("GS=%.3f|VE=%.3f,%.3f,%.3f|WF=%d|FOO=%d|MCH=%.3f|OTD=%.3f|OTC=%d|FON=%d|OBC=%d"),
		QueryConfig.GridSpacing,
		QueryConfig.ValidationExtent.X,
		QueryConfig.ValidationExtent.Y,
		QueryConfig.ValidationExtent.Z,
		QueryConfig.bApplyWorldFilter ? 1 : 0,
		QueryConfig.bFilterUnderOverhangs ? 1 : 0,
		QueryConfig.MinClearHeightAbove,
		QueryConfig.OverhangTraceDistance,
		static_cast<int32>(QueryConfig.OverhangTraceChannel),
		QueryConfig.bFilterOnObstacles ? 1 : 0,
		static_cast<int32>(QueryConfig.ObstacleTraceChannel)
	);
}

FDirectNavSamplerConfig UDirectNavPointExporterSubsystem::MakeSamplerConfig(const FDirectNavReachablePointQueryConfig& QueryConfig)
{
	FDirectNavSamplerConfig Config;
	Config.GridSpacing = QueryConfig.GridSpacing;
	Config.ValidationExtent = QueryConfig.ValidationExtent;
	return Config;
}

FDirectNavWorldFilterConfig UDirectNavPointExporterSubsystem::MakeWorldFilterConfig(const FDirectNavReachablePointQueryConfig& QueryConfig)
{
	FDirectNavWorldFilterConfig Config;
	Config.bFilterUnderOverhangs = QueryConfig.bFilterUnderOverhangs;
	Config.MinClearHeightAbove = QueryConfig.MinClearHeightAbove;
	Config.OverhangTraceDistance = QueryConfig.OverhangTraceDistance;
	Config.OverhangTraceChannel = QueryConfig.OverhangTraceChannel;
	Config.bFilterOnObstacles = QueryConfig.bFilterOnObstacles;
	Config.ObstacleTraceChannel = QueryConfig.ObstacleTraceChannel;
	return Config;
}

int32 UDirectNavPointExporterSubsystem::FilterPointsWithWorldCollision(
	UWorld* World,
	FDirectNavSamplingResult& InOutResult,
	const FDirectNavWorldFilterConfig& FilterConfig)
{
	if (!World)
	{
		return 0;
	}

	int32 RemovedCount = 0;
	TArray<FDirectNavSamplePoint> FilteredPoints;
	FilteredPoints.Reserve(InOutResult.SamplePoints.Num());

	for (const FDirectNavSamplePoint& Point : InOutResult.SamplePoints)
	{
		if (!Point.bIsValid)
		{
			continue;
		}

		bool bKeepPoint = true;

		if (FilterConfig.bFilterUnderOverhangs)
		{
			FHitResult HitResult;
			FCollisionQueryParams QueryParams(NAME_None, true);
			QueryParams.bTraceComplex = false;

			const FVector TraceStart = Point.Location + FVector(0.0f, 0.0f, 1.0f);
			const FVector TraceEnd = TraceStart + FVector(0.0f, 0.0f, FilterConfig.OverhangTraceDistance);

			if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, FilterConfig.OverhangTraceChannel, QueryParams))
			{
				const float ClearHeight = HitResult.Location.Z - Point.Location.Z;
				if (ClearHeight < FilterConfig.MinClearHeightAbove)
				{
					bKeepPoint = false;
				}
			}
		}

		if (bKeepPoint && FilterConfig.bFilterOnObstacles)
		{
			FHitResult HitResult;
			FCollisionQueryParams QueryParams(NAME_None, true);
			QueryParams.bTraceComplex = false;

			const FVector DownTraceStart = Point.Location + FVector(0.0f, 0.0f, 50.0f);
			const FVector DownTraceEnd = Point.Location - FVector(0.0f, 0.0f, 50.0f);

			if (World->LineTraceSingleByChannel(HitResult, DownTraceStart, DownTraceEnd, FilterConfig.ObstacleTraceChannel, QueryParams))
			{
				const float SurfaceDelta = FMath::Abs(HitResult.Location.Z - Point.Location.Z);
				if (SurfaceDelta > 10.0f)
				{
					bKeepPoint = false;
				}
			}
		}

		if (bKeepPoint)
		{
			FilteredPoints.Add(Point);
		}
		else
		{
			++RemovedCount;
		}
	}

	InOutResult.SamplePoints = MoveTemp(FilteredPoints);
	InOutResult.ValidPoints = InOutResult.SamplePoints.Num();
	return RemovedCount;
}
