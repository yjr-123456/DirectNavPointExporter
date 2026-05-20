#include "DirectNavPointExporterBlueprintLibrary.h"

#include "DirectNavPointExporterRuntime.h"
#include "DirectNavPointSampler.h"
#include "DirectNavPointExporterSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

bool UDirectNavPointExporterBlueprintLibrary::SampleFreePointsFromWorldNavMesh(
	UObject* WorldContextObject,
	const FDirectNavSamplerConfig& SamplerConfig,
	bool bApplyWorldFilter,
	const FDirectNavWorldFilterConfig& WorldFilterConfig,
	TArray<FVector>& OutPoints,
	FDirectNavSamplingResult& OutResult)
{
	OutPoints.Empty();
	OutResult.Reset();

	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World))
	{
		return false;
	}

	FDirectNavPointSampler Sampler;
	if (!Sampler.Initialize(World))
	{
		return false;
	}

	if (!Sampler.SampleGridPoints(SamplerConfig, OutResult))
	{
		return false;
	}

	if (bApplyWorldFilter)
	{
		FilterPointsWithWorldCollision(World, OutResult, WorldFilterConfig);
	}

	OutPoints = GetValidPoints(OutResult);
	return true;
}

bool UDirectNavPointExporterBlueprintLibrary::SampleFreePointsInRegionFromWorldNavMesh(
	UObject* WorldContextObject,
	const FBox& Region,
	const FDirectNavSamplerConfig& SamplerConfig,
	bool bApplyWorldFilter,
	const FDirectNavWorldFilterConfig& WorldFilterConfig,
	TArray<FVector>& OutPoints,
	FDirectNavSamplingResult& OutResult)
{
	OutPoints.Empty();
	OutResult.Reset();

	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World))
	{
		return false;
	}

	FDirectNavPointSampler Sampler;
	if (!Sampler.Initialize(World))
	{
		return false;
	}

	if (!Sampler.SampleGridPointsInRegion(Region, SamplerConfig, OutResult))
	{
		return false;
	}

	if (bApplyWorldFilter)
	{
		FilterPointsWithWorldCollision(World, OutResult, WorldFilterConfig);
	}

	OutPoints = GetValidPoints(OutResult);
	return true;
}

bool UDirectNavPointExporterBlueprintLibrary::HasValidWorldNavMesh(UObject* WorldContextObject)
{
	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World))
	{
		return false;
	}

	FDirectNavPointSampler Sampler;
	return Sampler.Initialize(World);
}

bool UDirectNavPointExporterBlueprintLibrary::GetWorldNavMeshBounds(UObject* WorldContextObject, FBox& OutBounds)
{
	OutBounds = FBox(ForceInit);

	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World))
	{
		return false;
	}

	FDirectNavPointSampler Sampler;
	if (!Sampler.Initialize(World))
	{
		return false;
	}

	OutBounds = Sampler.GetNavMeshBounds();
	return OutBounds.IsValid != 0;
}

TArray<FVector> UDirectNavPointExporterBlueprintLibrary::GetValidPoints(const FDirectNavSamplingResult& Result)
{
	TArray<FVector> ValidPoints;
	ValidPoints.Reserve(Result.ValidPoints);

	for (const FDirectNavSamplePoint& Point : Result.SamplePoints)
	{
		if (Point.bIsValid)
		{
			ValidPoints.Add(Point.Location);
		}
	}

	return ValidPoints;
}

bool UDirectNavPointExporterBlueprintLibrary::IsFreePointCacheReady(UObject* WorldContextObject)
{
	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		return Subsystem->IsCacheReady();
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::GetReachablePointsCached(
	UObject* WorldContextObject,
	const FDirectNavReachablePointQueryConfig& QueryConfig,
	TArray<FVector>& OutPoints,
	FDirectNavSamplingResult& OutResult)
{
	OutPoints.Empty();
	OutResult.Reset();

	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		return Subsystem->GetReachablePointsCached(QueryConfig, OutPoints, OutResult);
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::BuildReachableAreaCached(
	UObject* WorldContextObject,
	const FDirectNavReachablePointQueryConfig& QueryConfig,
	FDirectNavReachableAreaData& OutArea)
{
	OutArea.Reset();

	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		TArray<FVector> CachedPoints;
		FDirectNavSamplingResult CachedResult;
		if (!Subsystem->GetReachablePointsCached(QueryConfig, CachedPoints, CachedResult))
		{
			return false;
		}

		BuildReachableAreaData(
			CachedPoints,
			CachedResult,
			Subsystem->GetCacheStatus().MapName,
			FMath::Max(QueryConfig.GridSpacing, 1.0f),
			OutArea
		);
		return OutArea.CellCount > 0;
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::ShowReachableAreaCached(
	UObject* WorldContextObject,
	const FDirectNavReachablePointQueryConfig& QueryConfig,
	const FDirectNavReachableAreaDisplayConfig& DisplayConfig,
	int32& OutDrawnCellCount)
{
	OutDrawnCellCount = 0;

	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World))
	{
		return false;
	}

	FDirectNavReachableAreaData AreaData;
	if (!BuildReachableAreaCached(WorldContextObject, QueryConfig, AreaData))
	{
		return false;
	}

	OutDrawnCellCount = DrawReachableArea(World, AreaData, DisplayConfig);
	return OutDrawnCellCount > 0;
}

bool UDirectNavPointExporterBlueprintLibrary::GetDefaultCachedFreePointsFromWorldNavMesh(
	UObject* WorldContextObject,
	TArray<FVector>& OutPoints,
	FDirectNavSamplingResult& OutResult)
{
	OutPoints.Empty();
	OutResult.Reset();

	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		if (!Subsystem->IsCacheReady())
		{
			if (!Subsystem->ResamplePoints() || !Subsystem->IsCacheReady())
			{
				return false;
			}
		}

		OutPoints = Subsystem->GetCachedPoints();
		OutResult = Subsystem->GetCachedResult();
		return true;
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::ShowDefaultCachedReachableArea(
	UObject* WorldContextObject,
	const FDirectNavReachableAreaDisplayConfig& DisplayConfig,
	int32& OutDrawnCellCount)
{
	OutDrawnCellCount = 0;

	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World))
	{
		return false;
	}

	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		if (!Subsystem->IsCacheReady())
		{
			if (!Subsystem->ResamplePoints() || !Subsystem->IsCacheReady())
			{
				return false;
			}
		}

		FDirectNavReachableAreaData AreaData;
		const FDirectNavReachablePointQueryConfig DefaultQueryConfig = Subsystem->GetDefaultQueryConfig();
		BuildReachableAreaData(
			Subsystem->GetCachedPoints(),
			Subsystem->GetCachedResult(),
			Subsystem->GetCacheStatus().MapName,
			FMath::Max(DefaultQueryConfig.GridSpacing, 1.0f),
			AreaData
		);

		OutDrawnCellCount = DrawReachableArea(World, AreaData, DisplayConfig);
		return OutDrawnCellCount > 0;
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::RefreshFreePointCache(UObject* WorldContextObject)
{
	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		return Subsystem->RefreshContext() && Subsystem->ResamplePoints();
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::InvalidateFreePointCache(UObject* WorldContextObject, bool bInvalidateContext)
{
	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		Subsystem->InvalidateCache(bInvalidateContext);
		return true;
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::ConfigureFreePointAutoSampling(
	UObject* WorldContextObject,
	float StartupDelay,
	const FDirectNavSamplerConfig& SamplerConfig,
	bool bApplyWorldFilter,
	const FDirectNavWorldFilterConfig& WorldFilterConfig)
{
	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		Subsystem->SetSamplingOptions(StartupDelay, SamplerConfig, bApplyWorldFilter, WorldFilterConfig);
		Subsystem->ScheduleAutomaticSampling();
		return true;
	}

	return false;
}

bool UDirectNavPointExporterBlueprintLibrary::GetFreePointCacheStatus(UObject* WorldContextObject, FDirectNavCacheStatus& OutStatus)
{
	OutStatus = FDirectNavCacheStatus{};

	if (UDirectNavPointExporterSubsystem* Subsystem = GetDirectNavPointExporterSubsystem(WorldContextObject))
	{
		OutStatus = Subsystem->GetCacheStatus();
		return true;
	}

	return false;
}

UDirectNavPointExporterSubsystem* UDirectNavPointExporterBlueprintLibrary::GetDirectNavPointExporterSubsystem(UObject* WorldContextObject)
{
	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World) || !World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UDirectNavPointExporterSubsystem>();
}

bool UDirectNavPointExporterBlueprintLibrary::ClearReachableAreaDebug(UObject* WorldContextObject)
{
	UWorld* World = nullptr;
	if (!ResolveWorld(WorldContextObject, World))
	{
		return false;
	}

	FlushPersistentDebugLines(World);
	return true;
}

void UDirectNavPointExporterBlueprintLibrary::BuildReachableAreaData(
	const TArray<FVector>& Points,
	const FDirectNavSamplingResult& Result,
	const FString& MapName,
	float GridSpacing,
	FDirectNavReachableAreaData& OutArea)
{
	OutArea.Reset();
	OutArea.MapName = MapName;
	OutArea.GridSpacing = FMath::Max(GridSpacing, 1.0f);

	if (Points.IsEmpty())
	{
		return;
	}

	const float EffectiveGridSpacing = OutArea.GridSpacing;
	const float HalfSpacing = EffectiveGridSpacing * 0.5f;
	const FVector BoundsOrigin = Result.NavMeshBounds.IsValid ? Result.NavMeshBounds.Min : Points[0];
	TSet<FIntPoint> SeenCells;
	OutArea.CellCenters.Reserve(Points.Num());

	for (const FVector& Point : Points)
	{
		const int32 GridX = FMath::RoundToInt((Point.X - BoundsOrigin.X) / EffectiveGridSpacing);
		const int32 GridY = FMath::RoundToInt((Point.Y - BoundsOrigin.Y) / EffectiveGridSpacing);
		const FIntPoint CellIndex(GridX, GridY);
		if (SeenCells.Contains(CellIndex))
		{
			continue;
		}

		SeenCells.Add(CellIndex);
		const FVector SnappedCenter(
			BoundsOrigin.X + GridX * EffectiveGridSpacing,
			BoundsOrigin.Y + GridY * EffectiveGridSpacing,
			Point.Z
		);
		OutArea.CellCenters.Add(SnappedCenter);
		OutArea.CoveredBounds += SnappedCenter + FVector(HalfSpacing, HalfSpacing, 0.0f);
		OutArea.CoveredBounds += SnappedCenter - FVector(HalfSpacing, HalfSpacing, 0.0f);
	}

	OutArea.CellCount = OutArea.CellCenters.Num();
}

int32 UDirectNavPointExporterBlueprintLibrary::DrawReachableArea(
	UWorld* World,
	const FDirectNavReachableAreaData& AreaData,
	const FDirectNavReachableAreaDisplayConfig& DisplayConfig)
{
	if (!World || AreaData.CellCenters.IsEmpty())
	{
		return 0;
	}

	const int32 MaxCells = DisplayConfig.MaxCellsToDraw <= 0
		? AreaData.CellCenters.Num()
		: FMath::Min(DisplayConfig.MaxCellsToDraw, AreaData.CellCenters.Num());
	const float HalfSpacing = FMath::Max(AreaData.GridSpacing * 0.5f, 1.0f);
	const FVector CellExtent(HalfSpacing, HalfSpacing, 2.0f);
	const FColor DrawColor = DisplayConfig.Color.ToFColor(true);
	const bool bPersistent = DisplayConfig.bPersistent;
	const float Lifetime = bPersistent ? -1.0f : FMath::Max(DisplayConfig.Duration, 0.0f);

	int32 DrawnCellCount = 0;
	for (int32 CellIndex = 0; CellIndex < MaxCells; ++CellIndex)
	{
		const FVector ElevatedCenter = AreaData.CellCenters[CellIndex] + FVector(0.0f, 0.0f, DisplayConfig.HeightOffset);

		if (DisplayConfig.bDrawFilledCells)
		{
			DrawDebugSolidBox(World, ElevatedCenter, CellExtent, DrawColor, bPersistent, Lifetime);
		}

		if (DisplayConfig.bDrawCellOutline)
		{
			DrawDebugBox(World, ElevatedCenter, CellExtent, DrawColor, bPersistent, Lifetime, 0, DisplayConfig.LineThickness);
		}

		++DrawnCellCount;
	}

	return DrawnCellCount;
}

int32 UDirectNavPointExporterBlueprintLibrary::FilterPointsWithWorldCollision(
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

bool UDirectNavPointExporterBlueprintLibrary::ResolveWorld(UObject* WorldContextObject, UWorld*& OutWorld)
{
	OutWorld = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!OutWorld)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Failed to resolve world from context object"));
		return false;
	}

	return true;
}
