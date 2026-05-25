# DirectNavPointExporter API

## Overview
`DirectNavPointExporterRuntime` provides runtime access to NavMesh-based free point sampling, cached reachable-point queries, radius filtering, and debug visualization.

Module: `DirectNavPointExporterRuntime`

## Data Types

### `FDirectNavSamplePoint`
- `FVector Location`
- `bool bIsValid`

### `FDirectNavSamplerConfig`
- `float GridSpacing`
- `FVector ValidationExtent`

### `FDirectNavWorldFilterConfig`
- `bool bFilterUnderOverhangs`
- `float MinClearHeightAbove`
- `float OverhangTraceDistance`
- `ECollisionChannel OverhangTraceChannel`
- `bool bFilterOnObstacles`
- `ECollisionChannel ObstacleTraceChannel`

### `FDirectNavSamplingResult`
- `TArray<FDirectNavSamplePoint> SamplePoints`
- `int32 TotalPoints`
- `int32 ValidPoints`
- `FBox NavMeshBounds`
- `float TotalArea`
- `void Reset()`

### `FDirectNavReachablePointQueryConfig`
- `float GridSpacing`
- `FVector ValidationExtent`
- `bool bApplyWorldFilter`
- `bool bFilterUnderOverhangs`
- `float MinClearHeightAbove`
- `float OverhangTraceDistance`
- `ECollisionChannel OverhangTraceChannel`
- `bool bFilterOnObstacles`
- `ECollisionChannel ObstacleTraceChannel`

### `FDirectNavCacheStatus`
- `bool bContextReady`
- `bool bHasDefaultResult`
- `FString MapName`
- `int32 CachedQueryCount`
- `int32 CachedPointCount`
- `int32 ContextVersion`

### `FDirectNavReachableAreaData`
- `TArray<FVector> CellCenters`
- `float GridSpacing`
- `FBox CoveredBounds`
- `int32 CellCount`
- `FString MapName`
- `void Reset()`

### `FDirectNavReachableAreaDisplayConfig`
- `FLinearColor Color`
- `float Duration`
- `bool bPersistent`
- `float HeightOffset`
- `float LineThickness`
- `int32 MaxCellsToDraw`
- `bool bDrawFilledCells`
- `bool bDrawCellOutline`

### `FDirectNavRadiusPointQueryConfig`
- `FVector Center`
- `float Radius`
- `bool bUse2DDistance`

## `FDirectNavPointSampler`
Low-level Detour/Recast sampler.

### API
- `bool Initialize(UWorld* InWorld)`
- `bool IsValid() const`
- `FBox GetNavMeshBounds() const`
- `bool IsValidPoint(const FVector& Point, const FVector& Extent) const`
- `bool SampleGridPoints(const FDirectNavSamplerConfig& Config, FDirectNavSamplingResult& OutResult)`
- `bool SampleGridPointsInRegion(const FBox& Region, const FDirectNavSamplerConfig& Config, FDirectNavSamplingResult& OutResult)`

## `UDirectNavPointExporterSubsystem`
World subsystem that owns cache/context state.

### API
- `bool ResamplePoints()`
- `bool GetReachablePointsCached(const FDirectNavReachablePointQueryConfig& QueryConfig, TArray<FVector>& OutPoints, FDirectNavSamplingResult& OutResult)`
- `bool GetReachablePointsInRadiusCached(const FDirectNavReachablePointQueryConfig& QueryConfig, const FDirectNavRadiusPointQueryConfig& RadiusQuery, TArray<FVector>& OutPoints, FDirectNavSamplingResult& OutResult)`
- `void SetSamplingOptions(float InStartupDelay, const FDirectNavSamplerConfig& InSamplerConfig, bool bInApplyWorldFilter, const FDirectNavWorldFilterConfig& InWorldFilterConfig)`
- `void ScheduleAutomaticSampling()`
- `bool RefreshContext()`
- `void InvalidateCache(bool bInvalidateContext = false)`
- `void InvalidateForWorldChange()`
- `bool IsCacheReady() const`
- `const TArray<FVector>& GetCachedPoints() const`
- `const FDirectNavSamplingResult& GetCachedResult() const`
- `float GetStartupDelay() const`
- `FDirectNavCacheStatus GetCacheStatus() const`
- `FDirectNavReachablePointQueryConfig GetDefaultQueryConfig() const`

### Notes
- `ResamplePoints()` updates the default cached result.
- `GetReachablePointsInRadiusCached()` filters a cached reachable-point result by radius.
- Cache is world/map scoped and invalidated on world change.

## `UDirectNavPointExporterBlueprintLibrary`
Blueprint-facing API.

### Free point sampling
- `SampleFreePointsFromWorldNavMesh(...)`
- `SampleFreePointsInRegionFromWorldNavMesh(...)`
- `HasValidWorldNavMesh(...)`
- `GetWorldNavMeshBounds(...)`
- `GetValidPoints(...)`

### Cached reachable-point queries
- `IsFreePointCacheReady(...)`
- `GetReachablePointsCached(...)`
- `BuildReachableAreaCached(...)`
- `GetReachablePointsInRadiusCached(...)`

### Visualization
- `ShowReachableAreaCached(...)`
- `GetDefaultCachedFreePointsFromWorldNavMesh(...)`
- `GetDefaultCachedFreePointsInRadius(...)`
- `ShowDefaultCachedReachableArea(...)`
- `ClearReachableAreaDebug(...)`

### Cache lifecycle
- `RefreshFreePointCache(...)`
- `InvalidateFreePointCache(...)`
- `ConfigureFreePointAutoSampling(...)`
- `GetFreePointCacheStatus(...)`
- `GetDirectNavPointExporterSubsystem(...)`

### Visualization behavior
- `ShowReachableAreaCached(...)` and `ShowDefaultCachedReachableArea(...)` draw actual reachable points inside a radius.
- They do not draw the full cached set anymore.
- `BuildReachableAreaCached(...)` remains available for area data aggregation.

## UnrealCV Command Surface
The `UnrealCV` plugin exposes these commands through `ReachablePointsHandler`:

### Get

- `vget /reachablepoints [grid_spacing] [min_clear_height]`
- `vget /reachablepoints/inradius center_x center_y center_z radius [grid_spacing] [min_clear_height]`
- `vget /reachablepoints/count [grid_spacing] [min_clear_height]`
- `vget /reachablepoints/status`

### Set / debug
- `vset /reachablearea/show center_x center_y center_z radius [grid_spacing] [min_clear_height]`
- `vset /reachablearea/clear`
- `vset /reachablepoints/refresh`
- `vset /reachablepoints/invalidate [context]`

## Return Rules
- Most query/show functions return `false` on invalid world, invalid cache, or invalid radius.
- Radius queries require `Radius > 0`.
- `OutPoints` and `OutResult` are cleared before each query.

