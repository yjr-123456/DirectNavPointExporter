#pragma once

#include "CoreMinimal.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "DirectNavPointExporterTypes.h"

class UWorld;

class DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavPointSampler
{
public:
	FDirectNavPointSampler();
	~FDirectNavPointSampler();

	bool Initialize(UWorld* InWorld);
	bool IsValid() const { return NavMesh != nullptr && NavQuery != nullptr && QueryFilter != nullptr; }

	FBox GetNavMeshBounds() const;
	bool IsValidPoint(const FVector& Point, const FVector& Extent) const;
	bool SampleGridPoints(const FDirectNavSamplerConfig& Config, FDirectNavSamplingResult& OutResult);
	bool SampleGridPointsInRegion(const FBox& Region, const FDirectNavSamplerConfig& Config, FDirectNavSamplingResult& OutResult);

private:
	FVector RecastToUnreal(const dtReal RecastPoint[3]) const;
	void UnrealToRecast(const FVector& UnrealPoint, dtReal RecastPoint[3]) const;
	void CalculateBounds();
	void FindLargestConnectedRegion();
	bool IsInLargestRegion(dtPolyRef PolyRef) const;

private:
	const dtNavMesh* NavMesh;
	dtNavMeshQuery* NavQuery;
	dtQueryFilter* QueryFilter;
	FBox CachedBounds;
	TSet<dtPolyRef> GroundRegionPolys;
	bool bGroundRegionComputed;
};
